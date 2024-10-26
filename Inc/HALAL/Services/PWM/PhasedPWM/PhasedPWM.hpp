/*
 * PhasedPWMInstance.hpp
 *
 *  Created on: Feb 27, 2023
 *      Author: aleja
 */

#pragma once

#include "HALAL/Services/PWM/PWM/PWM.hpp"

#define STLIB_TIMER_CCMR_PWM_MODE_1 0x0
#define STLIB_TIMER_CCMR_PWM_MODE_2 (16+4096)
#define STLIB_TIMER_CCMR_REGISTER_MODE_MASK (0xFFFFFFFF - STLIB_TIMER_CCMR_PWM_MODE_2)

#define __STLIB_TIM_SET_MODE(__HANDLE__, __CHANNEL__, __CCMR_PWM_MODE_COMPARE__) \
	switch(__CHANNEL__){\
	case TIM_CHANNEL_1: \
	case TIM_CHANNEL_2: \
		(__HANDLE__)->Instance->CCMR1 &= STLIB_TIMER_CCMR_REGISTER_MODE_MASK;\
		(__HANDLE__)->Instance->CCMR1 |= __CCMR_PWM_MODE_COMPARE__;\
	break;\
	case TIM_CHANNEL_3: \
	case TIM_CHANNEL_4: \
		(__HANDLE__)->Instance->CCMR2 &= STLIB_TIMER_CCMR_REGISTER_MODE_MASK;\
		(__HANDLE__)->Instance->CCMR2 |= __CCMR_PWM_MODE_COMPARE__;\
	break;\
	case TIM_CHANNEL_5: \
	default:			\
		(__HANDLE__)->Instance->CCMR3 &= STLIB_TIMER_CCMR_REGISTER_MODE_MASK;\
		(__HANDLE__)->Instance->CCMR3 |= __CCMR_PWM_MODE_COMPARE__;\
	break;\
	}


class PhasedPWM :  public PWM {
protected:
	float raw_phase{};
	PhasedPWM() = default;

public:
	void set_duty_cycle(float duty_cycle);
	void set_frequency(uint32_t frequency);
	void set_raw_phase(float phase);
	void set_phase(float phase_in_deg);
	float get_phase()const;
	PhasedPWM(Pin& pin);

};
