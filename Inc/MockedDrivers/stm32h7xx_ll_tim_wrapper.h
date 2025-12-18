#ifndef STM32H7xx_LL_TIM_WRAPPER_H
#define STM32H7xx_LL_TIM_WRAPPER_H

#include <stddef.h>

#include "MockedDrivers/stm32h723xx_wrapper.h"

#include "stm32h7xx.h"

#include "MockedDrivers/mocked_ll_tim.hpp"

// -Woverflow does not do much in stm32h7xx_ll_tim.h
// the only places where it's used in are in WRITE_REG(reg, ~(bit))
// where they should use CLEAR_BIT(reg, bit)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverflow"
#define uint32_t size_t
#include "stm32h7xx_ll_tim.h"
#undef uint32_t
#pragma GCC diagnostic pop

#include "MockedDrivers/common.hpp"

#endif // STM32H7xx_LL_TIM_WRAPPER_H
