/*
 * SPI.hpp
 *
 *  Created on: 5 nov. 2022
 *      Author: Pablo
 */

#include "HALAL/Services/Communication/SPI/SPI.hpp"

#include "HALAL/Models/MPUManager/MPUManager.hpp"

#ifdef HAL_SPI_MODULE_ENABLED

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

    // this three lines don t do nothing yet, as the alternative_function on a
    // pin is hardcoded on the hal_msp.c
    spi_instance->SCK->alternative_function = AF5;
    spi_instance->MOSI->alternative_function = AF5;
    spi_instance->MISO->alternative_function = AF5;

    spi_instance->SPIOrderID =
        (uint16_t*)MPUManager::allocate_non_cached_memory(32);
    spi_instance->available_end =
        (uint16_t*)MPUManager::allocate_non_cached_memory(32);
    spi_instance->rx_buffer = (uint8_t*)MPUManager::allocate_non_cached_memory(
        SPI_MAXIMUM_PACKET_SIZE_BYTES);
    spi_instance->tx_buffer = (uint8_t*)MPUManager::allocate_non_cached_memory(
        SPI_MAXIMUM_PACKET_SIZE_BYTES);

    Pin::inscribe(*spi_instance->SCK, ALTERNATIVE);
    Pin::inscribe(*spi_instance->MOSI, ALTERNATIVE);
    Pin::inscribe(*spi_instance->MISO, ALTERNATIVE);

    if (spi_instance->mode == SPI_MODE_MASTER) {
        Pin::inscribe(*spi_instance->SS, OUTPUT);
    }

    uint8_t id = SPI::id_counter++;
    if (spi_instance->use_DMA) {
        static constexpr auto dma_spi1_config {DMA::inscribe_stream<SPI1_BASE, DMA1_Stream3_BASE, DMA1_Stream4_BASE>()};
    }
    SPI::registered_spi[id] = spi_instance;
    SPI::registered_spi_by_handler[spi_instance->hspi] = spi_instance;

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
        HAL_GPIO_WritePin(iter.second->SS->port, iter.second->SS->gpio_pin,
                          (GPIO_PinState)PinState::ON);
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

    SPI::Instance* spi = SPI::registered_spi[id];
    turn_off_chip_select(spi);
    HAL_StatusTypeDef errorcode =
        HAL_SPI_Transmit(spi->hspi, data.data(), data.size(), 10);
    turn_on_chip_select(spi);
    switch (errorcode) {
        case HAL_OK:
            return true;
            break;
        case HAL_BUSY:
            return false;
            break;
        default:
            ErrorHandler(
                "Error while transmiting and receiving with spi DMA. Errorcode "
                "%u",
                (uint8_t)errorcode);
            return false;
            break;
    }
}

bool SPI::transmit_DMA(uint8_t id, span<uint8_t> data) {
    if (!SPI::registered_spi.contains(id)) {
        ErrorHandler("No SPI registered with id %u", id);
        return false;
    }

    SPI::Instance* spi = SPI::registered_spi[id];

    HAL_StatusTypeDef errorcode =
        HAL_SPI_Transmit_DMA(spi->hspi, data.data(), data.size());
    turn_on_chip_select(spi);
    switch (errorcode) {
        case HAL_OK:
            return true;
            break;
        case HAL_BUSY:
            return false;
            break;
        default:
            ErrorHandler(
                "Error while transmiting and receiving with spi DMA. Errorcode "
                "%u",
                (uint8_t)errorcode);
            return false;
            break;
    }
}

bool SPI::receive(uint8_t id, span<uint8_t> data) {
    if (!SPI::registered_spi.contains(id)) {
        ErrorHandler("No SPI registered with id %u", id);
        return false;
    }

    SPI::Instance* spi = SPI::registered_spi[id];
    turn_off_chip_select(spi);
    HAL_StatusTypeDef errorcode =
        HAL_SPI_Receive(spi->hspi, data.data(), data.size(), 10);
    turn_on_chip_select(spi);
    switch (errorcode) {
        case HAL_OK:
            return true;
            break;
        case HAL_BUSY:
            return false;
            break;
        default:
            ErrorHandler(
                "Error while transmiting and receiving with spi DMA. Errorcode "
                "%u",
                (uint8_t)errorcode);
            return false;
            break;
    }
}
bool SPI::receive_DMA(uint8_t id, span<uint8_t> data) {
    if (!SPI::registered_spi.contains(id)) {
        ErrorHandler("No SPI registered with id %u", id);
        return false;
    }

    SPI::Instance* spi = SPI::registered_spi[id];

    HAL_StatusTypeDef errorcode =
        HAL_SPI_Receive_DMA(spi->hspi, data.data(), data.size());
    switch (errorcode) {
        case HAL_OK:
            return true;
            break;
        case HAL_BUSY:
            return false;
            break;
        default:
            ErrorHandler(
                "Error while transmiting and receiving with spi DMA. Errorcode "
                "%u",
                (uint8_t)errorcode);
            return false;
            break;
    }
}

bool SPI::transmit_and_receive(uint8_t id, span<uint8_t> command_data,
                               span<uint8_t> receive_data) {
    if (!SPI::registered_spi.contains(id)) {
        ErrorHandler("No SPI registered with id %u", id);
        return false;
    }

    SPI::Instance* spi = SPI::registered_spi[id];
    turn_off_chip_select(spi);
    HAL_StatusTypeDef errorcode = HAL_SPI_Transmit(
        spi->hspi, command_data.data(), command_data.size(), 10);
    if (errorcode != HAL_OK) {
        return false;
    }
    if (HAL_SPI_Receive(spi->hspi, receive_data.data(), receive_data.size(),
                        10) != HAL_OK) {
        turn_on_chip_select(spi);
        ErrorHandler("Error during receive in %s", spi->name.c_str());
        return false;
    }
    turn_on_chip_select(spi);
    return true;
}
bool SPI::transmit_and_receive_DMA(uint8_t id, span<uint8_t> command_data,
                                   span<uint8_t> receive_data) {
    if (!SPI::registered_spi.contains(id)) {
        ErrorHandler("No SPI registered with id %u", id);
        return false;
    }

    SPI::Instance* spi = SPI::registered_spi[id];

    HAL_StatusTypeDef errorcode =
        HAL_SPI_TransmitReceive_DMA(spi->hspi, command_data.data(),
                                    receive_data.data(), command_data.size());
    switch (errorcode) {
        case HAL_OK:
            return true;
            break;
        case HAL_BUSY:
            return false;
            break;
        default:
            ErrorHandler(
                "Error while transmiting and receiving with spi DMA. Errorcode "
                "%u",
                (uint8_t)errorcode);
            return false;
            break;
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
                    *(spi->SPIOrderID) = spi->SPIOrderQueue.last();
                    spi->SPIOrderQueue.pop();
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

    spi->hspi->Instance = spi->instance;
    spi->hspi->Init.Mode = spi->mode;
    spi->hspi->Init.Direction = SPI_DIRECTION_2LINES;
    spi->hspi->Init.DataSize = spi->data_size;
    spi->hspi->Init.CLKPolarity = spi->clock_polarity;
    spi->hspi->Init.CLKPhase = spi->clock_phase;

    if (spi->mode == SPI_MODE_MASTER) {
        spi->hspi->Init.NSS = SPI_NSS_SOFT;
        spi->hspi->Init.BaudRatePrescaler = spi->baud_rate_prescaler;
    } else {
        spi->hspi->Init.NSS = SPI_NSS_SOFT;
    }

    spi->hspi->Init.FirstBit = spi->first_bit;
    spi->hspi->Init.TIMode = SPI_TIMODE_DISABLE;
    spi->hspi->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    spi->hspi->Init.CRCPolynomial = 0x0;
    spi->hspi->Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
    spi->hspi->Init.NSSPolarity = spi->nss_polarity;
    spi->hspi->Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
    spi->hspi->Init.TxCRCInitializationPattern =
        SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
    spi->hspi->Init.RxCRCInitializationPattern =
        SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
    spi->hspi->Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
    spi->hspi->Init.MasterInterDataIdleness =
        SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
    spi->hspi->Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
    spi->hspi->Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
    spi->hspi->Init.IOSwap = SPI_IO_SWAP_DISABLE;

    if (HAL_SPI_Init(spi->hspi) != HAL_OK) {
        ErrorHandler("Unable to init %s", spi->name);
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
    HAL_SPI_TransmitReceive_DMA(spi->hspi, value_to_send, value_to_receive,
                                size_to_send);
}

void SPI::turn_on_chip_select(SPI::Instance* spi) {
    HAL_GPIO_WritePin(spi->SS->port, spi->SS->gpio_pin,
                      (GPIO_PinState)PinState::ON);
}

void SPI::turn_off_chip_select(SPI::Instance* spi) {
    HAL_GPIO_WritePin(spi->SS->port, spi->SS->gpio_pin,
                      (GPIO_PinState)PinState::OFF);
}

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
