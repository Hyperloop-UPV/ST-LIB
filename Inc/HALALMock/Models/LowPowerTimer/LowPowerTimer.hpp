/*
 * LowPowerTimer.hpp
 *
 *  Created on: Nov 23, 2022
 *      Author: alejandro
 */

#pragma once
#include <map>
#include <cstring>
#include <string>


#include "stm32h7xx_hal.h"

#ifdef HAL_LPTIM_MODULE_ENABLED

using std::reference_wrapper;
using std::map;
using std::string;

class LowPowerTimer {
public:
	LPTIM_TypeDef& instance;
	LPTIM_HandleTypeDef& handle;
	uint16_t period;
	string name;

	LowPowerTimer() = default;
	LowPowerTimer(LPTIM_TypeDef& instance, LPTIM_HandleTypeDef& handle, uint16_t period, string name) :
		instance(instance), handle(handle), period(period), name(name) {};

	void init();
};

#endif
