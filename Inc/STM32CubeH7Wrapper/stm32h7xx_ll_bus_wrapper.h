#ifndef STM32H7xx_LL_BUS_WRAPPER_H
#define STM32H7xx_LL_BUS_WRAPPER_H

#ifndef SIM_ON
#include "stm32h7xx_ll_bus.h"
#else
#include "MockedDrivers/stm32h7xx_hal_mock.h"
#endif

#endif // STM32H7xx_LL_BUS_WRAPPER_H
