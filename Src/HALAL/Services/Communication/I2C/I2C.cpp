/*
 * I2C.hpp
 *
 *  Created on: 12 dec. 2022
 *      Author: Pablo, Ricardo
 */
#include "HALAL/Services/Communication/I2C/I2C.hpp"

#ifdef HAL_I2C_MODULE_ENABLED

unordered_map<uint8_t, I2C::Instance*> I2C::active_i2c;

uint16_t I2C::id_counter = 0;

uint8_t I2C::inscribe(I2C::Peripheral& i2c, uint8_t address) {
    if (!I2C::available_i2cs.contains(i2c)) {
        ErrorHandler("The I2C %d is not available on the runes files", (uint16_t)i2c);
        return 0;
    }

    I2C::Instance* i2c_instance = I2C::available_i2cs[i2c];

    Pin::inscribe(i2c_instance->SCL, ALTERNATIVE);
    Pin::inscribe(i2c_instance->SDA, ALTERNATIVE);

    DMA::inscribe_stream(i2c_instance->RX_DMA);
    DMA::inscribe_stream(i2c_instance->TX_DMA);

    uint8_t id = I2C::id_counter++;

    I2C::active_i2c[id] = i2c_instance;
    i2c_instance->address = address;

    return id;
}

uint8_t I2C::inscribe(I2C::Peripheral& i2c) { return I2C::inscribe(i2c, 0); }

void I2C::start() {
    for (auto iter : I2C::active_i2c) {
        I2C::init(iter.second);
    }
}

bool I2C::transmit_next_packet(uint8_t id, I2CPacket& packet) {
    if (!I2C::active_i2c.contains(id)) {
        ErrorHandler("I2C id not found on transmit packet \n\r");
        return false;
    }

    I2C::Instance* i2c = I2C::active_i2c[id];

    if (i2c->hi2c->State == HAL_I2C_STATE_BUSY_TX) {
        ErrorHandler("I2C Transmit buffer busy!\n\r");
        return false;
    }

    if (hdma_i2c2_tx.State != 0x01U) {
    }

    if (HAL_I2C_Master_Transmit_DMA(
            i2c->hi2c,
            packet.get_id() << 1,
            packet.get_data(),
            packet.get_size()
        ) != HAL_OK) {
        ErrorHandler("I2C Error during memory read DMA!\n\r");
        return false;
    }
    return true;
}

bool I2C::transmit_next_packet_polling(uint8_t id, I2CPacket& packet) {
    if (!I2C::active_i2c.contains(id)) {
        ErrorHandler("I2C id not found on transmit packet \n\r");
        return false;
    }

    I2C::Instance* i2c = I2C::active_i2c[id];

    if (i2c->hi2c->State == HAL_I2C_STATE_BUSY_TX) {
        ErrorHandler("I2C Transmit buffer busy!\n\r");
        return false;
    }

    if (HAL_I2C_Master_Transmit(
            i2c->hi2c,
            packet.get_id() << 1,
            packet.get_data(),
            packet.get_size(),
            50
        ) != HAL_OK) {
        // ErrorHandler("Error during I2C transmission \n\r");
        return false;
    }
    return true;
}

bool I2C::receive_next_packet(uint8_t id, I2CPacket& packet) {
    if (!I2C::active_i2c.contains(id)) {
        ErrorHandler("I2C id not found on receive packet \n\r");
        return false;
    }

    I2C::Instance* i2c = I2C::active_i2c[id];

    if (i2c->hi2c->State == HAL_I2C_STATE_BUSY_RX) {
        ErrorHandler("I2C Receive buffer busy!\n\r");
        return false;
    }

    *packet.get_data() = 0;

    if (HAL_I2C_Master_Receive_DMA(
            i2c->hi2c,
            packet.get_id() << 1,
            packet.get_data(),
            packet.get_size()
        ) != HAL_OK) {
        ErrorHandler("I2C Error during memory write DMA!\n\r");
        return false;
    }

    i2c->is_receive_ready = false;
    return true;
}

bool I2C::receive_next_packet_polling(uint8_t id, I2CPacket& packet) {
    if (!I2C::active_i2c.contains(id)) {
        ErrorHandler("I2C id not found on receive packet \n\r");
        return false;
    }

    I2C::Instance* i2c = I2C::active_i2c[id];

    if (i2c->hi2c->State == HAL_I2C_STATE_BUSY_RX) {
        ErrorHandler("I2C Receive buffer busy!\n\r");
        return false;
    }

    *packet.get_data() = 0;

    if (HAL_I2C_Master_Receive(
            i2c->hi2c,
            packet.get_id() << 1,
            packet.get_data(),
            packet.get_size(),
            50
        ) != HAL_OK) {
        // ErrorHandler("I2C Error during receive!\n\r");
        return false;
    }

    i2c->is_receive_ready = false;
    return true;
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef* hi2c) {
    auto result = find_if(I2C::active_i2c.begin(), I2C::active_i2c.end(), [&](auto i2c) {
        return (i2c.second->hi2c == hi2c);
    });

    if (result != I2C::active_i2c.end()) {
        (*result).second->is_receive_ready = true;
    } else {
        // TODO: Warning: Data receive form an unknown SPI
    }
}

bool I2C::read_from(uint8_t id, I2CPacket& packet, uint16_t mem_addr, uint16_t mem_size) {
    if (!I2C::active_i2c.contains(id)) {
        ErrorHandler("I2C id not found on read \n\r");
        return false;
    }

    I2C::Instance* i2c = I2C::active_i2c[id];

    *packet.get_data() = 0;

    if (i2c->hi2c->State == HAL_I2C_STATE_BUSY_TX) {
        ErrorHandler("I2C Transmit buffer busy!\n\r");
        return false;
    }

    if (HAL_I2C_Mem_Read_DMA(
            i2c->hi2c,
            packet.get_id() << 1,
            mem_addr,
            1,
            packet.get_data(),
            packet.get_size()
        )) {
        ErrorHandler("I2C Error during memory read DMA!\n\r");
        return false;
    }

    i2c->is_receive_ready = false;
    return true;
}

bool I2C::write_to(uint8_t id, I2CPacket& packet, uint16_t mem_addr, uint16_t mem_size) {
    if (!I2C::active_i2c.contains(id)) {
        ErrorHandler("I2C id not found on write \n\r");
        return false;
    }

    I2C::Instance* i2c = I2C::active_i2c[id];

    if (i2c->hi2c->State == HAL_I2C_STATE_BUSY_RX) {
        ErrorHandler("I2C Receive buffer busy!\n\r");
        return false;
    }

    if (HAL_I2C_Mem_Write_DMA(
            i2c->hi2c,
            packet.get_id() << 1,
            mem_addr,
            mem_size,
            packet.get_data(),
            packet.get_size()
        )) {
        ErrorHandler("I2C Error during memory write DMA!\n\r");
        return false;
    }

    return true;
}

bool I2C::has_next_packet(uint8_t id) {
    if (!I2C::active_i2c.contains(id)) {
        ErrorHandler("I2C id not found on next packet check \n\r");
        return false;
    }

    I2C::Instance* i2c = I2C::active_i2c[id];

    return i2c->is_receive_ready;
}

bool I2C::is_busy(uint8_t id) {
    if (!I2C::active_i2c.contains(id)) {
        ErrorHandler("I2C id not found on is busy check \n\r");
        return false;
    }
    I2C::Instance* i2c = I2C::active_i2c[id];

    if (i2c->hi2c->State == HAL_I2C_STATE_BUSY_TX) {
        return true;
    }

    return false;
}

void I2C::reset(uint8_t id) {
    I2C::Instance* i2c = I2C::active_i2c[id];
    HAL_I2C_DeInit(i2c->hi2c);
    HAL_I2C_Init(i2c->hi2c);
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef* hspi) {
    // TODO: Fault, I2C error
    return;
}

void I2C::init(I2C::Instance* i2c) {
    if (i2c->is_initialized) {
        return;
    }
    if (!available_speed_frequencies.contains(i2c->speed_frequency_kHz)) {
        ErrorHandler("Error initializing, the frequency of the I2C is not available");
        return;
    }
    i2c->hi2c->Instance = i2c->instance;
    i2c->hi2c->Init.Timing = available_speed_frequencies[i2c->speed_frequency_kHz];
    i2c->hi2c->Init.OwnAddress1 = (i2c->address) << 1;
    i2c->hi2c->Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    i2c->hi2c->Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    i2c->hi2c->Init.OwnAddress2 = 0;
    i2c->hi2c->Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    i2c->hi2c->Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    i2c->hi2c->Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(i2c->hi2c) != HAL_OK) {
        ErrorHandler("Error configurating I2C");
    }

    if (HAL_I2CEx_ConfigAnalogFilter(i2c->hi2c, I2C_ANALOGFILTER_ENABLE) != HAL_OK) {
        ErrorHandler("Error configurating Analog Filter of the I2C");
    }

    if (HAL_I2CEx_ConfigDigitalFilter(i2c->hi2c, 0) != HAL_OK) {
        ErrorHandler("Error configurating Digital Filter of the I2C");
    }
    i2c->is_initialized = true;
}
#endif