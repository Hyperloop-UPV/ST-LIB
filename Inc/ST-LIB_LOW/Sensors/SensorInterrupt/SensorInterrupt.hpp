
/*
 * SensorInterrupt.hpp
 *
 *  Created on: Jan 31, 2023
 *      Author: ricardo
 */

#pragma once
#include "HALAL/HALAL.hpp"
#include "ErrorHandler/ErrorHandler.hpp"

class SensorInterrupt{
public:
	SensorInterrupt() = default;
	SensorInterrupt(Pin &pin, std::function<void()> &&action, PinState *value, TRIGGER trigger = TRIGGER::RISING_EDGE);
	SensorInterrupt(Pin &pin, std::function<void()> &&action, PinState &value, TRIGGER trigger = TRIGGER::RISING_EDGE);
	void read();
	uint8_t get_id();

protected:
	uint8_t id;
	PinState *value;
};
