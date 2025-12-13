#ifndef STM32H723xx_WRAPPER_H
#define STM32H723xx_WRAPPER_H

#include "stm32h723xx.h"

#undef RCC

RCC_TypeDef RCC_struct;
RCC_TypeDef *RCC = &RCC_struct;

#endif // STM32H723xx_WRAPPER_H
