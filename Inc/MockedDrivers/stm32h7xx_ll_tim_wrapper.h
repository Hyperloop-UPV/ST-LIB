#ifndef STM32H7xx_LL_TIM_WRAPPER_H
#define STM32H7xx_LL_TIM_WRAPPER_H

#include <stddef.h>

#include "MockedDrivers/stm32h723xx_wrapper.h"

#include "stm32h7xx.h"

#include "MockedDrivers/mocked_ll_tim.hpp"
#define uint32_t size_t
#include "stm32h7xx_ll_tim.h"
#undef uint32_t

#include "MockedDrivers/compiler_specific.hpp"

#endif // STM32H7xx_LL_TIM_WRAPPER_H
