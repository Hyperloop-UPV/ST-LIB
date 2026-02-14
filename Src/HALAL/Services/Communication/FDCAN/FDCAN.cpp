/*
 *  FDCAN.hpp
 *
 *  Created on: 5 nov. 2022
 *      Author: Pablo
 */

#include "HALAL/Services/Communication/FDCAN/FDCAN.hpp"

#ifdef HAL_FDCAN_MODULE_ENABLED

uint16_t FDCAN::id_counter = 0;

unordered_map<uint8_t, FDCAN::Instance*> FDCAN::registered_fdcan = {};

unordered_map<FDCAN::DLC, uint8_t> FDCAN::dlc_to_len = {
    {DLC::BYTES_0, 0},
    {DLC::BYTES_1, 1},
    {DLC::BYTES_2, 2},
    {DLC::BYTES_3, 3},
    {DLC::BYTES_4, 4},
    {DLC::BYTES_5, 5},
    {DLC::BYTES_6, 6},
    {DLC::BYTES_7, 7},
    {DLC::BYTES_8, 8},
    {DLC::BYTES_12, 12},
    {DLC::BYTES_16, 16},
    {DLC::BYTES_20, 20},
    {DLC::BYTES_24, 24},
    {DLC::BYTES_32, 32},
    {DLC::BYTES_48, 48},
    {DLC::BYTES_64, 64}
};
unordered_map<FDCAN_HandleTypeDef*, uint8_t> FDCAN::handle_to_id{};
unordered_map<FDCAN::Instance*, uint8_t> FDCAN::instance_to_id{};
FDCAN::Packet packet{.rx_data = array<uint8_t, 64>{}, .data_length = FDCAN::BYTES_64};

void FDCAN::start() {
    for (std::pair<uint8_t, FDCAN::Instance*> inst : FDCAN::registered_fdcan) {
        uint8_t id = inst.first;
        FDCAN::Instance* instance = inst.second;
        FDCAN::init(instance);

        instance->rx_queue = queue<FDCAN::Packet>();
        instance->tx_data = vector<uint8_t>();

        if (HAL_FDCAN_Start(instance->hfdcan) != HAL_OK) {
            ErrorHandler("Error during FDCAN %d initialization.", instance->fdcan_number);
        }

        if (HAL_FDCAN_ActivateNotification(instance->hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) !=
            HAL_OK) {
            ErrorHandler("Error activating FDCAN %d notifications.", instance->fdcan_number);
        }

        instance->start = true;

        FDCAN::registered_fdcan[id] = instance;
        FDCAN::instance_to_id[instance] = id;
    }
}

bool FDCAN::transmit(uint8_t id, uint32_t message_id, const char* data, FDCAN::DLC dlc) {
    if (not FDCAN::registered_fdcan.contains(id)) {
        ErrorHandler("There is no registered FDCAN with id: %d.", id);
        return false;
    }

    FDCAN::Instance* instance = registered_fdcan[id];

    if (not instance->start) {
        ErrorHandler("The FDCAN %d is not initialized.", instance->fdcan_number);
        return false;
    }

    instance->tx_header.Identifier = message_id;

    if (dlc != FDCAN::DLC::DEFAULT) {
        instance->tx_header.DataLength = dlc;
    }

    HAL_StatusTypeDef error =
        HAL_FDCAN_AddMessageToTxFifoQ(instance->hfdcan, &instance->tx_header, (uint8_t*)data);

    if (error != HAL_OK) {
        ErrorHandler(
            "Error sending message with id: 0x%x by FDCAN %d",
            message_id,
            instance->fdcan_number
        );
        return false;
    }

    return true;
}

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef* hfdcan, uint32_t RxFifo0ITs) {}

bool FDCAN::read(uint8_t id, FDCAN::Packet* data) {
    if (not FDCAN::registered_fdcan.contains(id)) {
        ErrorHandler("There is no FDCAN registered with id: %d.", id);
        return false;
    }

    if (!FDCAN::received_test(id)) {
        return false;
    }
    FDCAN_RxHeaderTypeDef header_buffer = FDCAN_RxHeaderTypeDef();
    HAL_FDCAN_GetRxMessage(
        FDCAN::registered_fdcan.at(id)->hfdcan,
        FDCAN::registered_fdcan.at(id)->rx_location,
        &header_buffer,
        data->rx_data.data()
    );
    if (data->identifier == FDCAN::ID::FAULT_ID) {
        ErrorHandler("FAULT PROPAGATED via CAN");
    }
    data->identifier = header_buffer.Identifier;
    data->data_length = static_cast<FDCAN::DLC>(header_buffer.DataLength);

    return true;
}

bool FDCAN::received_test(uint8_t id) {
    if (not FDCAN::registered_fdcan.contains(id)) {
        ErrorHandler("FDCAN with id %u not registered", id);
        return false;
    }

    return !((FDCAN::registered_fdcan.at(id)->hfdcan->Instance->RXF0S & FDCAN_RXF0S_F0FL) == 0U);
}

void FDCAN::init(FDCAN::Instance* fdcan) {
    FDCAN_HandleTypeDef* handle = fdcan->hfdcan;
    handle_to_id[handle] = instance_to_id[fdcan];

    if (HAL_FDCAN_Init(handle) != HAL_OK) {
        ErrorHandler("Error during FDCAN %d init.", fdcan->fdcan_number);
    }
}
#endif
