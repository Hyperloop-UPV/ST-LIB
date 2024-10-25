/*
 * ADC.hpp
 *
 *  Created on: 20 oct. 2022
 *      Author: alejandro
 */

#pragma once
#include <string>

#include "HALALMock/Models/PinModel/Pin.hpp"
#include "HALALMock/Models/LowPowerTimer/LowPowerTimer.hpp"
#include "HALALMock/Models/DMA/DMA.hpp"
#include "HALALMock/Services/SharedMemory/SharedMemory.hpp"

#if defined(HAL_ADC_MODULE_ENABLED) && defined(HAL_LPTIM_MODULE_ENABLED)

using std::string;

#define ADC_BUF_LEN 16
#define LPTIM1_PERIOD 6875
#define LPTIM2_PERIOD 6875
#define LPTIM3_PERIOD 6875

#define ADC_MAX_VOLTAGE 3.3
#define MAX_16BIT ((1<<16) - 1)
#define MAX_14BIT ((1<<14) - 1)
#define MAX_12BIT ((1<<12) - 1)
#define MAX_10BIT ((1<<10) - 1)



class ADC {
public:

	/// In STM32H723ZG, the ADC1 and ADC2 has 16 bits as their maximum resolution,
	/// while the ADC3 has 12 bits. Both of them can be configured to has less resolution than its maximum
	enum class ADCResolution : uint32_t {
		ADC_RES_16BITS = 0x00000000,
		ADC_RES_14BITS = 0x00000004,
		ADC_RES_12BITS = 0x00000008,
		ADC_RES_10BITS = 0x0000000C
	};

	struct InitData {
	public:
		ADC_TypeDef* adc;
		uint32_t resolution;
		/// @brief Resolution of the emulated ADC translated to an enum
		ADCResolution emulated_resolution;
		uint32_t external_trigger;
		vector<uint32_t> channels;
		DMA::Stream dma_stream;
		string name;

		InitData() = default;
		InitData(ADC_TypeDef* adc, uint32_t resolution, uint32_t external_trigger, vector<uint32_t>& channels, DMA::Stream dma_stream, string name);
	};

	class Peripheral {
	public:
		ADC_HandleTypeDef* handle;
		uint16_t* dma_data_buffer;
		LowPowerTimer timer;
		InitData init_data;
		bool is_on = false;

		Peripheral() = default;
		Peripheral(ADC_HandleTypeDef* handle, LowPowerTimer& timer, InitData& init_data);

		bool is_registered();
	};

	class Instance {
	public:
		Peripheral* peripheral;
		uint32_t channel;
		uint32_t rank;

		Instance() = default;
		Instance(Peripheral* peripheral, uint32_t channel);
	};


	/// @brief In this method we set the emulated pin as using as an ADC
	/// @param pin 
	/// @return Instance associated with that pin
	static uint8_t inscribe(Pin pin);

	/// @brief We duplicate association of instances to pines to emulated pins
	static void start();

	/// @brief As DMA has no sense in this mock, this method remains as it is to mantain the same interface, but nothing has been added
	static void turn_on(uint8_t id);

	/// @brief This method returns the voltage value of the emulated ADC
	/// @param id Instance from which the voltage will be returned
	/// @return Voltage value of the emulated ADC
	static float get_value(uint8_t id);

	/// @brief This method return the raw value of the emulated ADC, without any transformation to voltage
	/// @param id of the instance which we want to get the raw value
	/// @return Raw value of the emulated ADC
	static uint16_t get_int_value(uint8_t id);

	static uint16_t* get_value_pointer(uint8_t id);

	static Peripheral peripherals[3];

private:
	static uint32_t ranks[16];
	static map<Pin, Instance> available_instances;

	/** @brief To associate the emulated pins with the corresponding instances,
	 * as the same way as in available_instances. This map will be initialized
	 * with the same information as in available_instances in start method
	 */
	static map<Instance, EmulatedPin> available_emulated_instances; 

	static unordered_map<uint8_t, Instance> active_instances;

	static uint8_t id_counter;

	static void init(Peripheral& peripheral);
};

#endif
