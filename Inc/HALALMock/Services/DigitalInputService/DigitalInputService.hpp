/*
 * DigitalOut.hpp
 *
 *  Created on: Oct 17, 2022
 *      Author: stefan
 */
#pragma once
#include "HALALMock/Models/PinModel/Pin.hpp"
#include "HALALMock/Services/SharedMemory/SharedMemory.hpp"
#include "ErrorHandler/ErrorHandler.hpp"

class DigitalInput{
public:
	static map<uint8_t,Pin> service_ids;
	static uint8_t id_counter;

	static uint8_t inscribe(Pin& pin);
	static PinState read_pin_state(uint8_t id);
};
