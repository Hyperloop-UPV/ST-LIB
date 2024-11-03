/*
 * EXTI.cpp
 *
 *  Created on: Nov 5, 2022
 *      Author: alejandro 
 */

#include "HALALMock/Services/EXTI/EXTI.hpp"
#include "ErrorHandler/ErrorHandler.hpp"

uint8_t ExternalInterrupt::id_counter = 0;
map<uint8_t, Pin> ExternalInterrupt::service_ids = {};

ExternalInterrupt::Instance::Instance(uint32_t interrupt_request_number) :
		interrupt_request_number(interrupt_request_number) {}

void HAL_GPIO_EXTI_Callback (uint16_t GPIO_Pin) {
	ExternalInterrupt::Instance& exti = ExternalInterrupt::instances[GPIO_Pin];
	if (exti.is_on) {
		exti.action();
	}
}

uint8_t ExternalInterrupt::inscribe(Pin& pin, function<void()>&& action, TRIGGER trigger) {
	if (not instances.contains(pin.gpio_pin)) {
		ErrorHandler(" The pin %s is already used or isn t available for EXTI usage", pin.to_string().c_str());
		return 0;
	}
	EmulatedPin &pin_data = SharedMemory::get_pin(pin);
	if(pin_data.type != PinType::EXTIPin) {
		ErrorHandler("ID %d is not registered as a EXTIPin",id);
		return 0;
	}

	(pin_data.PinData.exti.trigger_mode) = trigger;

	service_ids[id_counter] = pin;
	instances[pin.gpio_pin].action = action;

	return id_counter++;
}
//TODO: assigne priority on the emulated pin
void ExternalInterrupt::start() {
	for(auto id_instance : instances) {
		Instance& instance = id_instance.second;
	}
}

void ExternalInterrupt::turn_on(uint8_t id) {
	if (not service_ids.contains(id)) {
		ErrorHandler("ID %d is not registered as an active instance", id);
		return;
	}

	Instance& instance = instances[service_ids[id].gpio_pin];
	Pin& pin = service_ids[id];
	EmulatedPin &pin_data = SharedMemory::get_pin(pin);
	if(pin_data.type != PinType::EXTIPin) {
		ErrorHandler("ID %d is not registered as a EXTIPin",id);
		return;
	}
	(pin_data.PinData.exti.is_on) = true;
}

bool ExternalInterrupt::get_pin_value(uint8_t id) {
	if (not service_ids.contains(id)) {
		ErrorHandler("ID %d is not registered as an active instance", id);
		return false;
	}

	Pin& pin = service_ids[id];
	EmulatedPin &pin_data = SharedMemory::get_pin(pin);
	if(pin_data.type != PinType::EXTIPin) {
		ErrorHandler("ID %d is not registered as a EXTIPin",id);
		std::terminate();
	}
	return (pin_data.PinData.exti.trigger_signal);
}
