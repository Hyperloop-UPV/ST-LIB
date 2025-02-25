/*
 * EXTI.hpp
 *
 *  Created on: Nov 5, 2022
 *      Author: alejandro 
 */

#pragma once
#include "HALALMock/Models/PinModel/Pin.hpp"
#include "HALALMock/Services/SharedMemory/SharedMemory.hpp"
#include <mutex>
#include <thread>
#include <vector>
#include <algorithm>


#define GPIO_PORT GPIOE

class ExternalInterrupt {
public:
	class Instance {
	public:
		//IRQn_Type is an enum nodeclared in sim
		uint32_t interrupt_request_number;

		function<void()> action = nullptr;
		bool is_on = false;
		bool *trigger_signal;

		Instance() = default;
		Instance(uint32_t interrupt_request_number);
	};

	static map<uint8_t, Pin> service_ids;
	static map<uint16_t, Instance> instances;
	static uint8_t id_counter;
	static bool is_running;
	static std::mutex mutex;

	static uint8_t inscribe(Pin& pin, function<void()>&& action, TRIGGER trigger=TRIGGER::RISING_EDGE);
	static void start();
	static void turn_on(uint8_t id);
	static void turn_off(uint8_t id);
	static bool get_pin_value(uint8_t id);
	static void stop();

private:
	static std::thread interrupt_thread;
	static std::vector<Instance*> interrupts_arr;
	static void handle_interrupts();
};
