
/*
 * DigitalSensor.hpp
 *
 *  Created on: Nov 2, 2022
 *      Author: ricardo
 */

#pragma once
#include "HALAL/HALAL.hpp"
#include "ErrorHandler/ErrorHandler.hpp"


class DigitalSensor{
public:
	DigitalSensor() = default;
	DigitalSensor(Pin &pin, PinState *value);
	DigitalSensor(Pin &pin, PinState &value);
	DigitalSensor(Pin &pin, PinState *value, OperationMode mode);
	DigitalSensor(Pin &pin, PinState &value, OperationMode mode);
	void read();
	uint8_t get_id();

protected:
	uint8_t id;
	PinState *value;
};


