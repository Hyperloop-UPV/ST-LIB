#ifndef STM32H7xx_LL_TIM_WRAPPER_H
#define STM32H7xx_LL_TIM_WRAPPER_H

#ifndef SIM_ON
#include "stm32h7xx_ll_tim.h"
#else
#include "MockedDrivers/mocked_ll_tim.hpp"
#endif

#endif // STM32H7xx_LL_TIM_WRAPPER_H
