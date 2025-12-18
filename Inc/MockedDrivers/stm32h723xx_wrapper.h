#ifndef STM32H723xx_WRAPPER_H
#define STM32H723xx_WRAPPER_H

#include <stdint.h>

#define STM32H723xx

#if defined(__GNUC__) || defined(__GNUG__)
# define __STATIC_INLINE static inline
#else
# error :)
#endif

#ifdef __cplusplus
  #define   __I     volatile             /*!< Defines 'read only' permissions */
#else
  #define   __I     volatile const       /*!< Defines 'read only' permissions */
#endif
#define __IO volatile

#define __RBIT __RBIT__CMSIS
#define TIM_TypeDef TIM_TypeDef__CMSIS

// only do __CORE_CM7_H_GENERIC in "core_cm7.h"
#define __CORE_CM7_H_DEPENDANT
#include "stm32h723xx.h"

#undef __RBIT
#undef TIM_TypeDef

#undef RCC
extern RCC_TypeDef *RCC;

#endif // STM32H723xx_WRAPPER_H
