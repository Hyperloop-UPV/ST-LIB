
/*
 * DigitalSensor.hpp
 *
 *  Created on: Nov 2, 2022
 *      Author: ricardo
 */

#pragma once
#include <cstdint>

#include "HALAL/Models/PinModel/Pin.hpp"


class DigitalSensor{
public:
	DigitalSensor() = default;
	DigitalSensor(Pin &pin, PinState *value);
	DigitalSensor(Pin &pin, PinState &value);
	void read();
	uint8_t get_id();

protected:
	uint8_t id;
	PinState *value;
};

