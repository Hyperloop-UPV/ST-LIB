/*
 * DigitalInput.cpp
 *
 *  Created on: Oct 18, 2022
 *      Author: stefan
 */

#include "HALALMock/Services/DigitalInputService/DigitalInputService.hpp"

uint8_t DigitalInput::id_counter = 0;
map<uint8_t,Pin> DigitalInput::service_ids = {};

uint8_t DigitalInput::inscribe(Pin& pin){
		Pin::inscribe(pin, INPUT);
		DigitalInput::service_ids[id_counter] = pin;
		return id_counter++;
}

PinState DigitalInput::read_pin_state(uint8_t id){
	if (not DigitalInput::service_ids.contains(id)){
		ErrorHandler("ID %d is not registered as a DigitalInput", id);
		return PinState::OFF;
	}

	Pin pin = DigitalInput::service_ids[id];
	EmulatedPin& pin_data = SharedMemory::get_pin(pin);
	return (PinState)pin_data.curr_state;
}
