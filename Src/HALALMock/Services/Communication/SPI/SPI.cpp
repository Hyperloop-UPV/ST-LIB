#include "HALALMock/Services/Communication/SPI/SPI.hpp"
#include "HALALMock/Models/MPUManager/MPUManager.hpp"
#include <HALALMock/Services/SharedMemory/SharedMemory.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#ifdef HAL_SPI_MODULE_ENABLED

#define SPI_PORT_BASE 2000

map<uint8_t, SPI::Instance*> SPI::registered_spi{};
map<SPI_HandleTypeDef*, SPI::Instance*> SPI::registered_spi_by_handler{};

uint16_t SPI::id_counter = 0;

/*=========================================
 * User functions for configuration of the SPI
 ==========================================*/

uint8_t SPI::inscribe(SPI::Peripheral& spi) {
    if (!SPI::available_spi.contains(spi)) {
        ErrorHandler(
            " The SPI peripheral %d is already used or does not exists.",
            (uint16_t)spi);

        return 0;
    }

    SPI::Instance* spi_instance = SPI::available_spi[spi];

    EmulatedPin &SCK_pin = SharedMemory::get_pin(*spi_instance->SCK);
    EmulatedPin &MOSI_pin = SharedMemory::get_pin(*spi_instance->MOSI);
    EmulatedPin &MISO_pin = SharedMemory::get_pin(*spi_instance->MISO);

    if (SCK_pin.type != PinType::NOT_USED || MOSI_pin.type != PinType::NOT_USED ||
        MISO_pin.type != PinType::NOT_USED) {
        ErrorHandler("The SPI pins are already used");
        return 0;
    }

    SCK_pin.type = PinType::SPI;
    MOSI_pin.type = PinType::SPI;
    MISO_pin.type = PinType::SPI;

    if (spi_instance->mode == SPI_MODE_MASTER) {
        EmulatedPin &SS_pin = SharedMemory::get_pin(*spi_instance->SS);

        if (SS_pin.type != PinType::NOT_USED) {
            ErrorHandler("The SPI SS pin is already used");
            return 0;
        }
    }

    uint8_t id = SPI::id_counter++;
    if (spi_instance->use_DMA) {
        DMA::inscribe_stream(spi_instance->hdma_rx);
        DMA::inscribe_stream(spi_instance->hdma_tx);
    }
    SPI::registered_spi[id] = spi_instance;
    SPI::registered_spi_by_handler[spi_instance->hspi] = spi_instance;

    // Create socket for this SPI peripheral
    int spi_socket = socket(AF_INET, SOCK_STREAM, 0);

    // Bind this socket to the local port 200x
    sockaddr_in local_address;
    local_address.sin_family = AF_INET;
    local_address.sin_port = htons(SPI_PORT_BASE + id_counter);
    local_address.sin_addr.s_addr = INADDR_ANY;

    if (bind(spi_socket, (struct sockaddr*)&local_address, sizeof(local_address)) < 0) {
        ErrorHandler("Bind error: %s", strerror(errno));
        close(spi_socket);
        return 0;
    }

    // Connect this peripheral with slave simulator
    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(2000 + id_counter);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (connect(spi_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        ErrorHandler("Connect error: %s", strerror(errno));
        close(spi_socket);
        return 0;
    }

    spi_sockets[id] = spi_socket;

    return id;
}

void SPI::assign_RS(uint8_t id, Pin& RSPin) {
    if (!SPI::registered_spi.contains(id)) {
        ErrorHandler("No SPI registered with id %u", id);
        return;
    }

    SPI::Instance* spi = SPI::registered_spi[id];
    spi->RS = &RSPin;
    if (spi->mode == SPI_MODE_MASTER) {
        spi->RShandler = DigitalInput::inscribe(RSPin);
    } else {
        spi->RShandler = DigitalOutputService::inscribe(RSPin);
    }
    spi->using_ready_slave = true;
}

void SPI::start() {
    for (auto iter : SPI::registered_spi) {
        SPI::init(iter.second);
    }
}

/*=========================================
 * User functions for dummy mode
 ==========================================*/

bool SPI::transmit(uint8_t id, uint8_t data) {
    array<uint8_t, 1> data_array = {data};
    return SPI::transmit(id, data_array);
}

bool SPI::transmit(uint8_t id, span<uint8_t> data) {
    if (!SPI::registered_spi.contains(id)) {
        ErrorHandler("No SPI registered with id %u", id);
        return false;
    }

    if(send(spi_sockets[id], data.data(), data.size(), 0) < 0) {
        ErrorHandler("Send error: %s", strerror(errno));
        return false;
    }

}

bool SPI::transmit_DMA(uint8_t id, span<uint8_t> data) {
    return SPI::transmit(id, data); // DMA is not used in simulator mode
}

bool SPI::receive(uint8_t id, span<uint8_t> data) {
    if (!SPI::registered_spi.contains(id)) {
        ErrorHandler("No SPI registered with id %u", id);
        return false;
    }

    if(recv(spi_sockets[id], data.data(), data.size(), 0) < 0) {
        ErrorHandler("Receive error: %s", strerror(errno));
        return false;
    }
}

bool SPI::transmit_and_receive(uint8_t id, span<uint8_t> command_data,
                               span<uint8_t> receive_data) {
    if (!SPI::registered_spi.contains(id)) {
        ErrorHandler("No SPI registered with id %u", id);
        return false;
    }

    if(SPI::transmit(id, command_data) && SPI::receive(id, receive_data)) {
        HAL_SPI_TxRxCpltCallback(SPI::registered_spi[id]->hspi);
        return true;
    } else {
        return false;
    }

}

/*=============================================
 * User functions for Order mode
 ==============================================*/

bool SPI::master_transmit_Order(uint8_t id, SPIBaseOrder& Order) {
    if (!SPI::registered_spi.contains(id)) {
        ErrorHandler("No SPI registered with id %u", id);
        return false;
    }

    SPI::Instance* spi = SPI::registered_spi[id];

    if (spi->state != SPI::IDLE || !spi->SPIOrderQueue.is_empty()) {
        return spi->SPIOrderQueue.push(Order.id);
    }

    spi->state = SPI::STARTING_ORDER;
    *(spi->SPIOrderID) = Order.id;
    master_check_available_end(spi);
    return true;
}

bool SPI::master_transmit_Order(uint8_t id, SPIBaseOrder* Order) {
    if (!SPI::registered_spi.contains(id)) {
        ErrorHandler("No SPI registered with id %u", id);
        return false;
    }

    SPI::Instance* spi = SPI::registered_spi[id];

    if (spi->state != SPI::IDLE || !spi->SPIOrderQueue.is_empty()) {
        return spi->SPIOrderQueue.push(Order->id);
    }

    spi->state = SPI::STARTING_ORDER;
    *(spi->SPIOrderID) = Order->id;
    master_check_available_end(spi);
    return true;
}

void SPI::slave_listen_Orders(uint8_t id) {
    if (!SPI::registered_spi.contains(id)) {
        ErrorHandler("No SPI registered with id %u", id);
        return;
    }

    SPI::Instance* spi = SPI::registered_spi[id];

    if (spi->state != SPI::IDLE) {
        return;
    }

    spi->state = SPI::WAITING_ORDER;
    *(spi->available_end) = NO_ORDER_ID;
    SPI::slave_check_packet_ID(spi);
}

/*=============================================
 * SPI Module Functions for HAL (cannot be private)
 ==============================================*/

void SPI::Order_update() {
    for (auto iter : SPI::registered_spi) {
        SPI::Instance* spi = iter.second;
        if (spi->mode == SPI_MODE_MASTER) {
            if (spi->state == SPI::IDLE) {
                if (!spi->SPIOrderQueue.is_empty()) {
                    spi->state = SPI::STARTING_ORDER;
                    *(spi->SPIOrderID) = spi->SPIOrderQueue.pop();
                    master_check_available_end(spi);
                }
            }

            // if the master is trying to start a Order transaction (he is
            // starting Order and the id of that Order is not 0)
            if (*spi->SPIOrderID != 0 && spi->state == STARTING_ORDER) {
                // when the slave available Order is not confirmed to be the
                // same Order id that the master is asking
                if (*spi->available_end != *spi->SPIOrderID) {
                    // enough time has passed since the last check to ask the
                    // slave again if it has the correct Order ID ready
                    if (known_slave_ready(spi)) {
                        master_check_available_end(spi);
                    } else if (Time::get_global_tick() - spi->last_end_check >
                               MASTER_SPI_CHECK_DELAY) {
                        master_check_available_end(spi);
                        spi->last_end_check = Time::get_global_tick();
                    }
                }
            }
        } else {
            spi_check_bus_collision(spi);
        }
    }
}

void SPI::master_check_available_end(SPI::Instance* spi) {
    SPI::turn_off_chip_select(spi);
    spi_communicate_order_data(spi, (uint8_t*)spi->SPIOrderID,
                               (uint8_t*)spi->available_end, 2);
}

void SPI::slave_check_packet_ID(SPI::Instance* spi) {
    spi_communicate_order_data(spi, (uint8_t*)spi->available_end,
                               (uint8_t*)spi->SPIOrderID, 2);
}

void SPI::chip_select_on(uint8_t id) {
    if (!SPI::registered_spi.contains(id)) {
        ErrorHandler("No SPI registered with id %u", id);
        return;
    }

    SPI::Instance* spi = SPI::registered_spi[id];
    turn_on_chip_select(spi);
}

void SPI::chip_select_off(uint8_t id) {
    if (!SPI::registered_spi.contains(id)) {
        ErrorHandler("No SPI registered with id %u", id);
        return;
    }

    SPI::Instance* spi = SPI::registered_spi[id];
    turn_off_chip_select(spi);
}

void SPI::init(SPI::Instance* spi) {
    if (spi->initialized) {
        return;
    }

    spi->initialized = true;
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef* hspi) {
    if (!SPI::registered_spi_by_handler.contains(hspi)) {
        ErrorHandler("Used SPI protocol without the HALAL SPI interface");
        return;
    }

    SPI::Instance* spi = SPI::registered_spi_by_handler[hspi];
    switch (spi->state) {
        case SPI::IDLE:
            // Does nothing as there is no Order handling on a direct send
            break;
        case SPI::STARTING_ORDER: {
            SPIBaseOrder* Order =
                SPIBaseOrder::SPIOrdersByID[*(spi->SPIOrderID)];
            if (spi->mode ==
                SPI_MODE_MASTER) {  // checks if the Order is ready on slave
                if (*(spi->available_end) == *(spi->SPIOrderID)) {
                    spi->state = SPI::PROCESSING_ORDER;
                    Order->master_prepare_buffer(spi->tx_buffer);
                    SPI::turn_off_chip_select(spi);

                    SPI::spi_communicate_order_data(
                        spi, spi->tx_buffer, spi->rx_buffer,
                        Order->payload_size - PAYLOAD_OVERHEAD);
                } else {
                    spi->try_count++;
                    switch (*(spi->available_end)) {
                        case NO_ORDER_ID: {
                            spi->last_end_check = Time::get_global_tick();
                            SPI::turn_on_chip_select(spi);
                        } break;
                        default:
                        case ERROR_ORDER_ID: {
                            spi->last_end_check = Time::get_global_tick();
                            spi->error_count++;
                            SPI::turn_on_chip_select(spi);
                        } break;
                    }
                }
            } else {
                ErrorHandler("Used master transmit Order on a slave spi");
            }
            break;
        }
        case SPI::WAITING_ORDER: {
            SPIBaseOrder* Order =
                SPIBaseOrder::SPIOrdersByID[*(spi->SPIOrderID)];
            if (Order == 0x0) {
                SPI::spi_recover(spi, hspi);
                return;
            } else if (spi->mode ==
                       SPI_MODE_SLAVE) {  // prepares the Order on the slave
                Order->slave_prepare_buffer(spi->tx_buffer);
                SPI::spi_communicate_order_data(
                    spi, spi->tx_buffer, spi->rx_buffer, Order->payload_size);
                SPI::mark_slave_ready(spi);
                spi->state = SPI::PROCESSING_ORDER;
            } else {
                ErrorHandler("Used slave process Orders on a master spi");
            }
            break;
        }
        case SPI::PROCESSING_ORDER: {
            SPIBaseOrder* Order =
                SPIBaseOrder::SPIOrdersByID[*(spi->SPIOrderID)];

            if (spi->mode == SPI_MODE_MASTER) {  // ends communication
                if (*(uint16_t*)&spi
                         ->rx_buffer[Order->CRC_index - PAYLOAD_OVERHEAD] !=
                    *spi->SPIOrderID) {
                    spi->state = SPI::STARTING_ORDER;
                    SPI::master_check_available_end(spi);
                    return;
                }
                spi->Order_count++;
                SPI::turn_on_chip_select(spi);
                Order->master_process_callback(spi->rx_buffer);
                spi->state = SPI::IDLE;
            } else {  // prepares the next received Order
                if (*(uint16_t*)&spi->rx_buffer[Order->CRC_index] !=
                    *spi->SPIOrderID) {
                    SPI::spi_recover(spi, hspi);
                    return;
                }
                spi->Order_count++;
                SPI::mark_slave_waiting(spi);
                *(spi->SPIOrderID) = NO_ORDER_ID;
                *(spi->available_end) = NO_ORDER_ID;
                Order->slave_process_callback(spi->rx_buffer);
                SPI::slave_check_packet_ID(spi);
                spi->state = SPI::WAITING_ORDER;
            }
            break;
        }
        case SPI::ERROR_RECOVERY: {
            if (spi->mode == SPI_MODE_MASTER) {
                // TODO
            } else {
                SPI::mark_slave_waiting(spi);
                spi->state =
                    SPI::WAITING_ORDER;  // prepares the next received Order
                *(spi->SPIOrderID) = NO_ORDER_ID;
                *(spi->available_end) = NO_ORDER_ID;
                SPI::slave_check_packet_ID(spi);
            }
            break;
        }
        default:
            ErrorHandler("Unknown spi state: %d", spi->state);
            break;
    }
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef* hspi) {}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef* hspi) {}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef* hspi) {
    if ((hspi->ErrorCode & HAL_SPI_ERROR_UDR) != 0) {
        SPI::spi_recover(SPI::registered_spi_by_handler[hspi], hspi);
    } else {
        ErrorHandler("SPI error number %u", hspi->ErrorCode);
    }
}

void SPI::spi_communicate_order_data(SPI::Instance* spi, uint8_t* value_to_send,
                                     uint8_t* value_to_receive,
                                     uint16_t size_to_send) {
    // Search for the id of the SPI instance
    uint8_t id;
    for (auto iter : registered_spi) {
        if (iter.second == spi) {
            id = iter.first;
            break;
        }
    }

    SPI::transmit_and_receive(id, span<uint8_t>(value_to_send, size_to_send), 
                              span<uint8_t>(value_to_receive, size_to_send));
}

void SPI::turn_on_chip_select(SPI::Instance* spi) {}

void SPI::turn_off_chip_select(SPI::Instance* spi) {}

void SPI::mark_slave_ready(SPI::Instance* spi) {
    if (spi->using_ready_slave) {
        DigitalOutputService::turn_on(spi->RShandler);
    }
    return;
}

void SPI::mark_slave_waiting(SPI::Instance* spi) {
    if (spi->using_ready_slave) {
        DigitalOutputService::turn_off(spi->RShandler);
    }
    return;
}

bool SPI::known_slave_ready(SPI::Instance* spi) {
    if (spi->using_ready_slave) {
        return DigitalInput::read_pin_state(spi->RShandler) == PinState::ON;
    }
    return false;
}

void SPI::spi_recover(SPI::Instance* spi, SPI_HandleTypeDef* hspi) {
    SPI::mark_slave_waiting(spi);
    HAL_SPI_Abort(hspi);
    spi->error_count = spi->error_count + 1;
    *(spi->SPIOrderID) = CASTED_ERROR_ORDER_ID;
    *(spi->available_end) = CASTED_ERROR_ORDER_ID;
    spi->state = ERROR_RECOVERY;
    SPI::slave_check_packet_ID(spi);
}

void SPI::spi_check_bus_collision(SPI::Instance* spi) {
    if (spi->hspi->State == HAL_SPI_STATE_READY) {
        SPI::spi_recover(spi, spi->hspi);
    }
}

#endif
