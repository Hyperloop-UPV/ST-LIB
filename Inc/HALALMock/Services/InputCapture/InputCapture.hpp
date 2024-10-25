/*
 * InputCapture.hpp
 *
 *  Created on: 30 oct. 2022
 *      Author: alejandro 
 */

#pragma once
#include "HALALMock/Services/SharedMemory/SharedMemory.hpp"

#ifdef HAL_TIM_MODULE_ENABLED

class InputCapture {
public:
	class Instance {
	public:
		uint8_t id;
		Pin pin;
		uint32_t *frequency;
		uint8_t *duty_cycle;
		bool is_active;

		Instance() = default;
		Instance(Pin& pin, void* peripheral, uint32_t channel_rising, uint32_t channel_falling);
	};
	

	static map<uint8_t, InputCapture::Instance> active_instances;
	static map<Pin, InputCapture::Instance> available_instances;
	static uint8_t id_counter;

	static uint8_t inscribe(Pin& pin);
	static void turn_on(uint8_t id);
	static void turn_off(uint8_t id);
	static uint32_t read_frequency(uint8_t id);
	static uint8_t read_duty_cycle(uint8_t id);
};

#endif