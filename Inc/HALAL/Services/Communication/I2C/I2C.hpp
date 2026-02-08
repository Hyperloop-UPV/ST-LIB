/*
 * I2C.hpp
 *
 *  Created on: 12 dec. 2022
 *      Author: Pablo, Ricardo
 */
#pragma once

#include "HALAL/Models/PinModel/Pin.hpp"
#include "C++Utilities/CppUtils.hpp"
#include "HALAL/Models/Packets/I2CPacket.hpp"
#include "ErrorHandler/ErrorHandler.hpp"
#include "HALAL/Models/DMA/DMA.hpp"

#define HALAL_I2C_EMPTY_ID (uint16_t)0

#ifdef HAL_I2C_MODULE_ENABLED

/**
 * @brief I2C service class. Abstracts the use of the UART service of the HAL library.
 * 
 */
class I2C {
private:
	/**
	 * @brief Struct which defines all data referring to I2C peripherals. It is
	 *        declared private in order to prevent unwanted use. Only
	 *        predefined instances should be used.
	 *
	 */
	struct Instance {
		Pin &SCL;
		Pin &SDA;
		I2C_HandleTypeDef *hi2c;
		I2C_TypeDef *instance;
		DMA::Stream RX_DMA;
		DMA::Stream TX_DMA;
		uint8_t address = 0;
		uint32_t speed_frequency_kHz = 100;
		uint32_t data_length_in_bits = 8;
		bool is_receive_ready = false;
		bool is_initialized = false;

	};

public:
	/**
	 * @brief Enum which abstracts the use of the Instance struct to facilitate the mocking of the HALAL.Struct
	 *
	 */
	enum Peripheral {
		peripheral2 = 2,
	};

public:
	static uint16_t id_counter;

	static unordered_map<uint8_t, I2C::Instance*> active_i2c;

	static unordered_map<I2C::Peripheral, I2C::Instance*> available_i2cs;

	static unordered_map<uint32_t, uint32_t> available_speed_frequencies;

	/**
	 * @brief I2C 2 wrapper enum of the STM32H723.
	 *
	 */
	static I2C::Peripheral i2c2;

	static DMA_HandleTypeDef hdma_i2c2_rx;
	static DMA_HandleTypeDef hdma_i2c2_tx;

	/**
	 * @brief I2C 2 instance of the STM32H723.
	 *
	 */
	static I2C::Instance instance2;

	/**
	 * @brief Registers a new I2C.
	 *
	 * @param uart I2C peripheral to register.
	 * @return uint8_t Id of the service.
	 */
	static uint8_t inscribe(I2C::Peripheral &i2c);
	static uint8_t inscribe(I2C::Peripheral &i2c, uint8_t address);

	/**
	 * @brief This method initializes all registered I2Cs. The peripherals
	 * 		  must be enrolled before calling this method.
	 *
	 */
	static void start();

	/**@brief	Transmits 1 RawPacket of any size by DMAs.
	 * 			Handles the packet size automatically.
	 * @param id Id of the I2C
	 * @param packet Packet to be send
	 * @return bool Returns true if the request to send the packet has been done
	 *            successfully. Returns false if the I2C is busy or a problem
	 *            has occurred.
	 */
	static bool transmit_next_packet(uint8_t id, I2CPacket &packet);
	static bool transmit_next_packet_polling(uint8_t id, I2CPacket &packet);

	/**
	 * @brief This method request the receive of a new RawPacket of any size by DMA.
	 *
	 * @param id Id of the I2C
	 * @param packet RawPacket in which the data will be stored
	 * @return bool Return true if the order to receive a new packet has been
	 *            processed correctly. Return false if the I2C is busy or a
	 *            problem has occurred.
	 */
	static bool receive_next_packet(uint8_t id, I2CPacket &packet);
	static bool receive_next_packet_polling(uint8_t id, I2CPacket &packet);

	static bool read_from(uint8_t id, I2CPacket &packet, uint16_t mem_addr,
			uint16_t mem_size);

	static bool write_to(uint8_t id, I2CPacket &packet, uint16_t mem_addr,
			uint16_t mem_size);

	/**
	 * @brief This method is used to check if the UART receive operation has finished and data is ready.
	 *
	 * @param id Id of the UART
	 * @return bool Return true if the packet is ready to use and false if not.
	 */
	static bool has_next_packet(uint8_t id);

	/**
	 * @brief This method is used to check if the UART transmit operations busy.
	 *
	 * @param id Id of the UART
	 * @return bool Return true if the UART transmit operation is busy and false if not.
	 */
	static bool is_busy(uint8_t id);

	/**
	 * @brief This method resets the i2c handler entirely so a board can recover from a communication fail without a full reset
	 */
	static void reset(uint8_t id);

private:
	/**
	 * @brief This method initializes the UART peripheral that is passed to it as a parameter.
	 *
	 * @param uart Peripheral instance to be initialized.
	 */
	static void init(I2C::Instance *i2c);
};

#endif