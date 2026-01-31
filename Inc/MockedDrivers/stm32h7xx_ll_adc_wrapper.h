#ifndef STM32H7xx_LL_ADC_WRAPPER_H
#define STM32H7xx_LL_ADC_WRAPPER_H

#include <stddef.h>

#include "MockedDrivers/stm32h723xx_wrapper.h"
//#include "MockedDrivers/mocked_ll_adc.hpp"

#define uint32_t size_t
#include "stm32h7xx_ll_adc.h"
#undef uint32_t

#include "MockedDrivers/common.hpp"

#endif // STM32H7xx_LL_ADC_WRAPPER_H
