/*
 * LowPowerTimer.cpp
 *
 *  Created on: 5 ene. 2023
 *      Author: aleja
 */

#include "HALALMock/Models/LowPowerTimer/LowPowerTimer.hpp"
#include "ErrorHandler/ErrorHandler.hpp"

void LowPowerTimer::init() {
	handle.Instance = &instance;
	handle.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
	handle.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV1;
	handle.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
	handle.Init.OutputPolarity = LPTIM_OUTPUTPOLARITY_HIGH;
	handle.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
	handle.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;
	handle.Init.Input1Source = LPTIM_INPUT1SOURCE_GPIO;
	handle.Init.Input2Source = LPTIM_INPUT2SOURCE_GPIO;

	if (HAL_LPTIM_Init(&handle) != HAL_OK) {
	  ErrorHandler("The LPTIM %s could not be registered", name);
	}
}
