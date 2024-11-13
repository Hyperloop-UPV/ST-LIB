/*
 * InputCapture.cpp
 *
 *  Created on: 30 oct. 2022
 *      Author: alejandro
 */
#include "HALALMock/Services/InputCapture/InputCapture.hpp"
#include "ErrorHandler/ErrorHandler.hpp"

uint8_t InputCapture::id_counter = 0;
map<uint8_t, InputCapture::Instance> InputCapture::active_instances = {};

InputCapture::Instance::Instance(Pin& pin, void* peripheral, uint32_t channel_rising, uint32_t channel_falling) :
	pin(pin)
	{
		EmulatedPin& sim_pin = SharedMemory::get_pin(pin);
		duty_cycle = &(sim_pin.PinData.input_capture.duty_cycle);
		frequency = &(sim_pin.PinData.input_capture.frequency);
		*duty_cycle = 0;
		*frequency = 0;
	}

uint8_t InputCapture::inscribe(Pin& pin){
 	if (not available_instances.contains(pin)) {
		ErrorHandler(" The pin %s is already used or isn t available for InputCapture usage", pin.to_string().c_str());
 		return 0;
 	}

	Pin::inscribe(pin, TIMER_ALTERNATE_FUNCTION);

 	Instance& data = available_instances[pin];
	active_instances[id_counter] = data;
	active_instances[id_counter].id = id_counter;

	return id_counter++;
}

void InputCapture::turn_on(uint8_t id){
	if (not active_instances.contains(id)) {
		ErrorHandler("ID %d is not registered as an active_instance", id);
		return;
	}
	Instance& instance = active_instances[id];
	instance.is_active = true;

}

void InputCapture::turn_off(uint8_t id){
	if (not active_instances.contains(id)) {
		ErrorHandler("ID %d is not registered as an active_instance", id);
		return;
	}
	Instance& instance = active_instances[id];
	instance.is_active = false;

}

uint32_t InputCapture::read_frequency(uint8_t id) {
	if (not active_instances.contains(id)) {
		ErrorHandler("ID %d is not registered as an active_instance", id);
		return 0;
	}
	Instance& instance = active_instances[id];
	if(!instance.is_active)
	{
		return 0;
	}
	
	return *instance.frequency;
}

uint8_t InputCapture::read_duty_cycle(uint8_t id) {
	if (not active_instances.contains(id)) {
		ErrorHandler("ID %d is not registered as an active_instance", id);
		return 0;
	}
	Instance& instance = active_instances[id];
	if(!instance.is_active)
	{
		return 0;
	}
	return *instance.duty_cycle;
}