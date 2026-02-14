#pragma once

#ifndef SIM_ON
#include "stm32h7xx_hal.h"
#else
#ifndef HAL_MODULE_ENABLED
#define HAL_MODULE_ENABLED
#endif
#ifndef HAL_GPIO_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#endif
#ifndef HAL_TIM_MODULE_ENABLED
#define HAL_TIM_MODULE_ENABLED
#endif
#ifndef HAL_ADC_MODULE_ENABLED
#define HAL_ADC_MODULE_ENABLED
#endif
#ifndef HAL_DMA_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#endif

#include "MockedDrivers/common.hpp"
#include "MockedDrivers/stm32h7xx_hal_mock.h"
#endif

#include "stm32h7xx_ll_adc_wrapper.h"
#include "stm32h7xx_ll_tim_wrapper.h"
#include "stm32h7xx_ll_gpio_wrapper.h"
#include "stm32h7xx_ll_bus_wrapper.h"

#ifdef SIM_ON
#ifndef SYSCFG_SWITCH_PC2
#define SYSCFG_SWITCH_PC2 SYSCFG_PMCR_PC2SO
#endif
#ifndef SYSCFG_SWITCH_PC3
#define SYSCFG_SWITCH_PC3 SYSCFG_PMCR_PC3SO
#endif
#ifndef SYSCFG_SWITCH_PC2_OPEN
#define SYSCFG_SWITCH_PC2_OPEN SYSCFG_PMCR_PC2SO
#endif
#ifndef SYSCFG_SWITCH_PC3_OPEN
#define SYSCFG_SWITCH_PC3_OPEN SYSCFG_PMCR_PC3SO
#endif

#ifndef assert_param
#define assert_param(expr) ((void)0U)
#endif

#ifndef TIM_EGR_UG
#if defined(TIM_EGR_UG_Msk)
#define TIM_EGR_UG TIM_EGR_UG_Msk
#else
// Mocked timer headers may not define update generation bit names.
#define TIM_EGR_UG (1U)
#endif
#endif

extern "C" void
HAL_SYSCFG_AnalogSwitchConfig(uint32_t SYSCFG_AnalogSwitch, uint32_t SYSCFG_SwitchState);
#endif

#ifndef STLIB_HAS_ADC3
#if defined(ADC3)
#define STLIB_HAS_ADC3 1
#else
#define STLIB_HAS_ADC3 0
#endif
#endif
