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
		EmulatedPin& emulated_pin = SharedMemory::get_pin(pin);
		if (emulated_pin.type != PinType::NOT_USED) {
		ErrorHandler("Pin %s is not available for ADC usage, is already using as %s", pin.to_string().c_str(), emulated_pin.type);
		return 0;
		}
		emulated_pin.type = PinType::DigitalInput;
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
	return (PinState)pin_data.PinData.digital_input.curr_state;
}
