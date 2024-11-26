#include "HALALMock/Services/Communication/SPI/SPI.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <thread>

#include "HALALMock/Services/SharedMemory/SharedMemory.hpp"

unordered_map<uint8_t, std::pair<int, int>> SPI::spi_master_sockets{};

map<uint8_t, SPI::Instance*> SPI::registered_spi{};

uint16_t SPI::id_counter = 0;

void sender_slave_thread(SPI::Instance& spi_instance) {
    // Prepare destination address
    sockaddr_in dest_address;
    dest_address.sin_family = AF_INET;  // Set IPv4
    dest_address.sin_port =
        htons(spi_instance.destination_address.second);  // Set port
    int result =
        inet_pton(AF_INET, spi_instance.destination_address.first,
                  &dest_address.sin_addr);  // Set broadcast IP, supposing that
                                            // whe have a mask = 255.255.255.0
    if (result <= 0) {
        if (result == 0) {
            ErrorHandler("Invalid broadcast address");
        } else {
            ErrorHandler("Error setting IP: %s", strerror(errno));
        }
    }

    // Init sending loop
    while (true) {
        // Wait for the queue to has data to send
        std::unique_lock lock(spi_instance.transmission_mx);
        spi_instance.cv_transmission.wait(lock, [&spi_instance] {
            return !spi_instance.transmission_queue.empty();
        });

        // Grab data to send from queue
        span<uint8_t> data = spi_instance.transmission_queue.front();

        // Send data
        if (sendto(spi_instance.socket, data.data(), data.size(), 0,
                   (struct sockaddr*)&dest_address, sizeof(dest_address)) < 0) {
            ErrorHandler("Send error: %s", strerror(errno));
        }

        // Erase data sended from queue
        spi_instance.transmission_queue.pop();
        lock.unlock();
    }
}

void sender_master_thread(SPI::Instance& spi_instance) {
    // Prepare destination address
    sockaddr_in dest_address;
    dest_address.sin_family = AF_INET;  // Set IPv4
    dest_address.sin_port =
        htons(spi_instance.destination_address.second);  // Set port
    int result =
        inet_pton(AF_INET, spi_instance.destination_address.first,
                  &dest_address.sin_addr);  // Set broadcast IP, supposing that
                                            // whe have a mask = 255.255.255.0
    if (result <= 0) {
        if (result == 0) {
            ErrorHandler("Invalid broadcast address");
        } else {
            ErrorHandler("Error setting IP: %s", strerror(errno));
        }
    }

    // Send SLAVE_SELECT command
    const char* select_command = "1";
    if (sendto(spi_instance.socket, select_command, strlen(select_command), 0,
               (struct sockaddr*)&dest_address, sizeof(dest_address)) < 0) {
        ErrorHandler("Send error: %s", strerror(errno));
    }

    // Init sending loop
    while (true) {
        // Wait for the queue to has data to send
        std::unique_lock lock(spi_instance.transmission_mx);
        spi_instance.cv_transmission.wait(lock, [&spi_instance] {
            return !spi_instance.transmission_queue.empty();
        });

        // Grab data to send from queue
        span<uint8_t> data = spi_instance.transmission_queue.front();

        // Add a '0' before data to indicate that is a normal packet
        std::vector<uint8_t> buffer;
        buffer.push_back(0);
        buffer.insert(buffer.end(), data.begin(), data.end());

        // Send data
        if (sendto(spi_instance.socket, buffer.data(), buffer.size(), 0,
                   (struct sockaddr*)&dest_address, sizeof(dest_address)) < 0) {
            ErrorHandler("Send error: %s", strerror(errno));
        }

        // Erase data sended from queue
        spi_instance.transmission_queue.pop();
        lock.unlock();
    }
}

void receiver_slave_thread(SPI::Instance& spi_instance) {
    std::unique_lock lock(spi_instance.reception_mx);
    span<uint8_t> data;

    while (true) {
        // Waiting to receive a command
        char command;
        if (recv(spi_instance.socket, &command, 1, MSG_WAITALL) < 0) {
            ErrorHandler("Receive error: %s", strerror(errno));
        }

        // Compute command
        switch (command) {
            case 0:  // Normal packet
                if (!spi_instance.selected) break;
                // Wait to the user to request a reception
                spi_instance.cv_reception.wait(lock, [&spi_instance] {
                    return !spi_instance.reception_queue.empty();
                });
                data = spi_instance.reception_queue.front();

                // Receive the packet
                if (recv(spi_instance.socket, data.data(), data.size(),
                         MSG_WAITALL) < 0) {
                    ErrorHandler("Receive error: %s", strerror(errno));
                }

                // Pop the request reception from the queue
                spi_instance.reception_queue.pop();

                lock.unlock();
                break;
            case 1:  // SLAVE_SELECT
                spi_instance.selected = true;
                break;
            case 2:  // SLAVE_DESELECT
                spi_instance.selected = false;
                break;
        }
    }
}

void receiver_master_thread(SPI::Instance& spi_instance) {
    std::unique_lock lock(spi_instance.reception_mx);

    while (true) {
        // Wait to the user to request a reception
        spi_instance.cv_reception.wait(lock, [&spi_instance] {
            return !spi_instance.reception_queue.empty();
        });
        span<uint8_t> data = spi_instance.reception_queue.front();

        // Receive the packet
        if (recv(spi_instance.socket, data.data(), data.size(), MSG_WAITALL) <
            0) {
            ErrorHandler("Receive error: %s", strerror(errno));
        }

        // Pop the request reception from the queue
        spi_instance.reception_queue.pop();

        lock.unlock();
    }
}

void sender_receiver_slave_thread(SPI::Instance& spi_instance,
                                  const uint8_t id, std::function<void(uint8_t)> callback) {
    // Prepare destination address
    sockaddr_in dest_address;
    dest_address.sin_family = AF_INET;  // Set IPv4
    dest_address.sin_port =
        htons(spi_instance.destination_address.second);  // Set port
    int result =
        inet_pton(AF_INET, spi_instance.destination_address.first,
                  &dest_address.sin_addr);  // Set broadcast IP, supposing that
                                            // whe have a mask = 255.255.255.0
    if (result <= 0) {
        if (result == 0) {
            ErrorHandler("Invalid broadcast address");
        } else {
            ErrorHandler("Error setting IP: %s", strerror(errno));
        }
    }

    // Init sending-receiving loop
    while (true) {
        // Wait the user to ask for a transmission-reception
        std::unique_lock tx_rx_lock(spi_instance.transmission_reception_mx);
        spi_instance.cv_transmission_reception.wait(tx_rx_lock, [&spi_instance] {
            return !spi_instance.transmission_reception_queue.empty();
        });

        // Wait this instance to be selected by the master
        std::unique_lock select_lock(spi_instance.selected_mx);
        spi_instance.cv_transmission_reception.wait(
            select_lock, [&spi_instance] { return &spi_instance.selected; });

        // Send data
        std::pair<span<uint8_t>, span<uint8_t>> data =
            spi_instance.transmission_reception_queue.front();
        if (sendto(spi_instance.socket, data.first.data(), data.first.size(), 0,
                   (struct sockaddr*)&dest_address, sizeof(dest_address)) < 0) {
            ErrorHandler("Send error: %s", strerror(errno));
        }

        // Receive data
        char command;
        if (recv(spi_instance.socket, &command, 1, MSG_WAITALL) < 0) {
            ErrorHandler("Receive error: %s", strerror(errno));
        }

        // Compute command
        switch (command) {
            case 0:  // Normal packet
                if (!spi_instance.selected) break;
                // Receive the packet
                if (recv(spi_instance.socket, data.second.data(), data.second.size(),
                         MSG_WAITALL) < 0) {
                    ErrorHandler("Receive error: %s", strerror(errno));
                }
                break;
            case 1:  // SLAVE_SELECT
                spi_instance.selected = true;
                break;
            case 2:  // SLAVE_DESELECT
                spi_instance.selected = false;
                break;
        }

        spi_instance.transmission_reception_queue.pop();
        select_lock.unlock();
        tx_rx_lock.unlock();

        callback(id);
    }
}

void sender_receiver_master_thread(SPI::Instance& spi_instance,
                                   const uint8_t id, std::function<void(uint8_t)> callback) {
    // Prepare destination address
    sockaddr_in dest_address;
    dest_address.sin_family = AF_INET;  // Set IPv4
    dest_address.sin_port =
        htons(spi_instance.destination_address.second);  // Set port
    int result =
        inet_pton(AF_INET, spi_instance.destination_address.first,
                  &dest_address.sin_addr);  // Set broadcast IP, supposing that
                                            // whe have a mask = 255.255.255.0
    if (result <= 0) {
        if (result == 0) {
            ErrorHandler("Invalid broadcast address");
        } else {
            ErrorHandler("Error setting IP: %s", strerror(errno));
        }
    }

    // Init sending-receiving loop
    while (true) {
        // Wait the user to ask for a transmission-reception
        std::unique_lock lock(spi_instance.transmission_reception_mx);
        spi_instance.cv_transmission_reception.wait(lock, [&spi_instance] {
            return !spi_instance.transmission_reception_queue.empty();
        });

        // Reach data from queue
        std::pair<span<uint8_t>, span<uint8_t>> data =
            spi_instance.transmission_reception_queue.front();

        // Add a '0' before data to indicate that is a normal packet
        std::vector<uint8_t> buffer;
        buffer.push_back(0);
        buffer.insert(buffer.end(), data.first.begin(), data.first.end());
        if (sendto(spi_instance.socket, data.first.data(), data.first.size(), 0,
                   (struct sockaddr*)&dest_address, sizeof(dest_address)) < 0) {
            ErrorHandler("Send error: %s", strerror(errno));
        }
        if (recv(spi_instance.socket, data.second.data(), data.second.size(),
                 MSG_WAITALL) < 0) {
            ErrorHandler("Receive error: %s", strerror(errno));
        }

        spi_instance.transmission_reception_queue.pop();
        lock.unlock();

        callback(id);
    }
}

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

    EmulatedPin& SCK_pin = SharedMemory::get_pin(*spi_instance->SCK);
    EmulatedPin& MOSI_pin = SharedMemory::get_pin(*spi_instance->MOSI);
    EmulatedPin& MISO_pin = SharedMemory::get_pin(*spi_instance->MISO);

    if (SCK_pin.type != PinType::NOT_USED ||
        MOSI_pin.type != PinType::NOT_USED ||
        MISO_pin.type != PinType::NOT_USED) {
        ErrorHandler("The SPI pins are already used");
        return 0;
    }

    SCK_pin.type = PinType::SPI;
    MOSI_pin.type = PinType::SPI;
    MISO_pin.type = PinType::SPI;

    uint8_t id = SPI::id_counter++;

    // Create socket for this SPI peripheral
    spi_instance->socket = socket(AF_INET, SOCK_DGRAM, 0);

    EmulatedPin& SS_pin = SharedMemory::get_pin(*spi_instance->SS);

    if (SS_pin.type != PinType::NOT_USED) {
        ErrorHandler("The SPI SS pin is already used");
        close(spi_instance->socket);
        return 0;
    }

    SS_pin.type = PinType::SPI;
    SS_pin.PinData.spi.is_on = false;  // When this pin turns on, the simulator
                                       // slave know that is selected.

    // Prepare local address
    sockaddr_in local_address;
    local_address.sin_family = AF_INET;                  // Set IPv4
    local_address.sin_port = htons(spi_instance->port);  // Set port
    int result =
        inet_pton(AF_INET, ip.c_str(), &local_address.sin_addr);  // Set IP
    if (result <= 0) {
        if (result == 0) {
            ErrorHandler("Invalid address");
        } else {
            ErrorHandler("Error setting IP: %s", strerror(errno));
        }
        close(spi_instance->socket);
        return 0;
    }

    // Bind this socket to the local port of the SPI peripheral
    if (bind(spi_instance->socket, (struct sockaddr*)&local_address,
             sizeof(local_address)) < 0) {
        ErrorHandler("Bind error: %s", strerror(errno));
        close(spi_instance->socket);
        return 0;
    }

    // Init sender and receiver threads
    if (spi_instance->mode == SPIMode::MASTER) {
        spi_instance->sender_thread = std::jthread([&spi_instance]() {
            sender_master_thread(*spi_instance);
        });
        spi_instance->receiver_thread = std::jthread([&spi_instance]() {
            receiver_master_thread(*spi_instance);
        });
        spi_instance->sender_receiver_thread =
            std::jthread([&spi_instance, &id]() {
            sender_receiver_master_thread(*spi_instance, id, &SPI::TxRxCpltCallback);
        });
    } else {
        spi_instance->sender_thread = std::jthread([&spi_instance]() {
            sender_slave_thread(*spi_instance);
        });
        spi_instance->receiver_thread = std::jthread([&spi_instance]() {
            receiver_slave_thread(*spi_instance);
        });
        spi_instance->sender_receiver_thread =
            std::jthread([&spi_instance, &id]() {
            sender_receiver_slave_thread(*spi_instance, id, &SPI::TxRxCpltCallback);
        });
    }

    SPI::registered_spi[id] = spi_instance;

    return id;
}

void SPI::assign_RS(uint8_t id, Pin& RSPin) {
    if (!SPI::registered_spi.contains(id)) {
        ErrorHandler("No SPI registered with id %u", id);
        return;
    }

    SPI::Instance* spi = SPI::registered_spi[id];
    spi->RS = &RSPin;
    if (spi->mode == SPIMode::MASTER) {
        spi->RShandler = DigitalInput::inscribe(RSPin);
    } else {
        spi->RShandler = DigitalOutputService::inscribe(RSPin);
    }
    spi->using_ready_slave = true;
}

void SPI::start() {
    for (auto [_, spi] : SPI::registered_spi) {
        SPI::init(spi);
        EmulatedPin& SS_pin = SharedMemory::get_pin(*spi->SS);
        SS_pin.PinData.spi.is_on =
            true;  // This pin tells the simulator to connect
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

    SPI::Instance* spi_instance(registered_spi[id]);

    std::unique_lock lock(spi_instance->transmission_mx);
    spi_instance->transmission_queue.push(data);
    lock.unlock();
    spi_instance->cv_transmission.notify_one();

    return true;
}

bool SPI::transmit_DMA(uint8_t id, span<uint8_t> data) {
    return SPI::transmit(id, data);  // DMA is not used in simulator mode
}

bool SPI::receive(uint8_t id, span<uint8_t> data) {
    if (!SPI::registered_spi.contains(id)) {
        ErrorHandler("No SPI registered with id %u", id);
        return false;
    }

    SPI::Instance* spi_instance(registered_spi[id]);

    std::unique_lock lock(spi_instance->reception_mx);
    spi_instance->reception_queue.push(data);
    lock.unlock();
    spi_instance->cv_reception.notify_one();

    return true;
}

bool SPI::transmit_and_receive(uint8_t id, span<uint8_t> command_data,
                               span<uint8_t> receive_data) {
    if (!SPI::registered_spi.contains(id)) {
        ErrorHandler("No SPI registered with id %u", id);
        return false;
    }

    SPI::Instance* spi_instance(registered_spi[id]);

    std::unique_lock lock(spi_instance->transmission_reception_mx);
    spi_instance->transmission_reception_queue.push(
        std::pair<span<uint8_t>, span<uint8_t>>(command_data, receive_data));
    lock.unlock();
    spi_instance->cv_transmission_reception.notify_one();

    return true;
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
        if (spi->mode == SPIMode::MASTER) {
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
                    // enough time has passed since the last check to ask
                    // the slave again if it has the correct Order ID ready
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

void SPI::TxRxCpltCallback(uint8_t id) {
    if (!SPI::registered_spi.contains(id)) {
        ErrorHandler("Used SPI protocol without the HALAL SPI interface");
        return;
    }

    SPI::Instance* spi = SPI::registered_spi[id];
    switch (spi->state) {
        case SPI::IDLE:
            // Does nothing as there is no Order handling on a direct send
            break;
        case SPI::STARTING_ORDER: {
            SPIBaseOrder* Order =
                SPIBaseOrder::SPIOrdersByID[*(spi->SPIOrderID)];
            if (spi->mode ==
                SPIMode::MASTER) {  // checks if the Order is ready on slave
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
                SPI::spi_recover(spi);
                return;
            } else if (spi->mode ==
                       SPIMode::SLAVE) {  // prepares the Order on the slave
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

            if (spi->mode == SPIMode::MASTER) {  // ends communication
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
                    SPI::spi_recover(spi);
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
            if (spi->mode == SPIMode::MASTER) {
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

void SPI::spi_communicate_order_data(SPI::Instance* spi, uint8_t* value_to_send,
                                     uint8_t* value_to_receive,
                                     uint16_t size_to_send) {
    // Search for the id of the SPI instance
    uint8_t id;
    for (auto [registered_id, registered_instance] : registered_spi) {
        if (registered_instance == spi) {
            id = registered_id;
            break;
        }
    }

    SPI::transmit_and_receive(id, span<uint8_t>(value_to_send, size_to_send),
                              span<uint8_t>(value_to_receive, size_to_send));
}

void SPI::turn_on_chip_select(SPI::Instance* spi) {
    EmulatedPin& SS_pin = SharedMemory::get_pin(*spi->SS);
    SS_pin.PinData.spi.is_on = true;
}

void SPI::turn_off_chip_select(SPI::Instance* spi) {
    EmulatedPin& SS_pin = SharedMemory::get_pin(*spi->SS);
    SS_pin.PinData.spi.is_on = false;
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

void SPI::spi_recover(SPI::Instance* spi) {
    SPI::mark_slave_waiting(spi);
    spi->error_count = spi->error_count + 1;
    *(spi->SPIOrderID) = CASTED_ERROR_ORDER_ID;
    *(spi->available_end) = CASTED_ERROR_ORDER_ID;
    spi->state = ERROR_RECOVERY;
    SPI::slave_check_packet_ID(spi);
}

void SPI::spi_check_bus_collision(SPI::Instance* spi) {}