#ifndef STM32H723xx_WRAPPER_H
#define STM32H723xx_WRAPPER_H

#include <stdint.h>
#ifdef __cplusplus
  #define   __I     volatile             /*!< Defines 'read only' permissions */
#else
  #define   __I     volatile const       /*!< Defines 'read only' permissions */
#endif
#define __IO volatile

#define __RBIT __RBIT__CMSIS
#define TIM_TypeDef TIM_TypeDef__CMSIS

// don't do anything in "core_cm7.h"
#define __CORE_CM7_H_GENERIC
#define __CORE_CM7_H_DEPENDANT
#include "stm32h723xx.h"

#undef __RBIT
#undef TIM_TypeDef

#undef RCC

extern RCC_TypeDef *RCC;

#endif // STM32H723xx_WRAPPER_H
