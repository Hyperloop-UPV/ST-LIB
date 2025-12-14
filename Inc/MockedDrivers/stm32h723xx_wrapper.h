#ifndef STM32H723xx_WRAPPER_H
#define STM32H723xx_WRAPPER_H

#include <stdint.h>
#ifdef __cplusplus
  #define   __I     volatile             /*!< Defines 'read only' permissions */
#else
  #define   __I     volatile const       /*!< Defines 'read only' permissions */
#endif
#define __IO volatile

// necessary remove of some warnings due to cmsis made for 32 bit arch and not 64 bit arch
#define __RBIT __RBIT__CMSIS
#define TIM_TypeDef TIM_TypeDef__CMSIS
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"

// don't do anything in "core_cm7.h"
#define __CORE_CM7_H_GENERIC
#define __CORE_CM7_H_DEPENDANT
#include "stm32h723xx.h"

#pragma GCC diagnostic pop
#undef __RBIT
#undef TIM_TypeDef

#undef RCC

extern RCC_TypeDef *RCC;

#endif // STM32H723xx_WRAPPER_H
