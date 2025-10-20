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
	uintptr_t instance;
	LPTIM_HandleTypeDef& handle;
	uint16_t period;
	const char* name;

	constexpr LowPowerTimer() = default;
	constexpr LowPowerTimer(uintptr_t instance, LPTIM_HandleTypeDef& handle, uint16_t period, const char* name) :
		instance(instance), handle(handle), period(period), name(name) {}
	void init();
};

#endif
