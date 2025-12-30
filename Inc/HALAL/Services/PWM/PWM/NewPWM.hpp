/*
 * NewPWM.hpp
 *
 *  Created on: Dec 30, 2025
 *      Author: victor
 */
#pragma once

#ifdef HAL_TIM_MODULE_ENABLED
#include "HALAL/Models/TimerDomain/TimerDomain.hpp"
#include "HALAL/Models/GPIO.hpp"

namespace ST_LIB {

template<const TimerDomain::Timer &dev>
struct TimerWrapper;

enum PWM_MODE : uint8_t {
    NORMAL = 0,
    PHASED = 1,
    CENTER_ALIGNED = 2,
};

struct PWMData {
    uint32_t channel;
    PWM_MODE mode;
};

template<const TimerDomain::Timer &dev>
class PWM {
    static constexpr size_t channel_to_CCR[7] = {
        0,
        offsetof(TIM_TypeDef, CCR1),
        offsetof(TIM_TypeDef, CCR2),
        offsetof(TIM_TypeDef, CCR3),
        offsetof(TIM_TypeDef, CCR4),
        offsetof(TIM_TypeDef, CCR5),
        offsetof(TIM_TypeDef, CCR6)
    };

    static constexpr uint32_t channel_to_channel_state_idx[7] = {
        0,
        0, 1, 2, 3, 4, 5,
    };

    static constexpr uint32_t channel_to_channelmul4[7] = {
        0,
        0x0, 0x4, 0x8, 0xC, 0x10, 0x14
    };

public:
    static constexpr float CLOCK_FREQ_MHZ_WITHOUT_PRESCALER = 275.0f;
    static constexpr float clock_period_ns = (1.0f/CLOCK_FREQ_MHZ_WITHOUT_PRESCALER)*1000.0f;

    TimerWrapper<dev> &timer;
    TimerPin pin;
    uint32_t frequency;
    float duty_cycle;
    bool is_on = false;
    bool is_center_aligned;

    PWM(TimerWrapper<dev> &tim, TimerPin pin) : timer(tim) {
        static_assert(dev.e.request != TimerRequest::Basic_6 && dev.e.request != TimerRequest::Basic_7, "These timers don't support pwm");
        this->is_center_aligned = ((tim.tim->CR1 & TIM_CR1_CMS) != 0);
        this->pin = pin;
    }

    void turn_on() {
        if(is_on) return;
        
        // if(HAL_TIM_PWM_Start(timer.htim, channel) != HAL_OK) { ErrorHandler("", 0); }
        HAL_TIM_ChannelStateTypeDef *state = &timer.htim.ChannelState[channel_to_channel_state_idx[pin.channel]];
        if(*state != HAL_TIM_CHANNEL_STATE_READY) {
            ErrorHandler("Channel not ready");
        }

        *state = HAL_TIM_CHANNEL_STATE_BUSY;
        // enable CCx
        uint32_t tmp = TIM_CCER_CC1E << (channel_to_channelmul4[pin.channel] & 0x1FU); /* 0x1FU = 31 bits max shift */

        SET_BIT(timer.tim->CCER, (uint32_t)(TIM_CCx_ENABLE << (channel_to_channelmul4[pin.channel] & 0x1FU)));

        // if timer supports break
        if constexpr ((dev.e.request == (TimerRequest)1) || (dev.e.request == (TimerRequest)8) || (dev.e.request == (TimerRequest)15) || (dev.e.request == (TimerRequest)16) || (dev.e.request == (TimerRequest)17))
        {
            // Main Output Enable
            SET_BIT(timer.tim->BDTR, TIM_BDTR_MOE);
        }

        // if timer can be a slave timer
        if constexpr ((dev.e.request == (TimerRequest)1) || (dev.e.request == (TimerRequest)2) || (dev.e.request == (TimerRequest)3) || (dev.e.request == (TimerRequest)4) || (dev.e.request == (TimerRequest)5) || (dev.e.request == (TimerRequest)8) || (dev.e.request == (TimerRequest)12) || (dev.e.request == (TimerRequest)15) || (dev.e.request == (TimerRequest)23) || (dev.e.request == (TimerRequest)24))
        {
            uint32_t tmpsmcr = timer.tim->SMCR & TIM_SMCR_SMS;
            if(!IS_TIM_SLAVEMODE_TRIGGER_ENABLED(tmpsmcr)) {
                timer.counter_enable();
            }
        } else {
            timer.counter_enable();
        }

        is_on = true;
    }

    void turn_off() {
        if(!is_on) return;
        // if(HAL_TIM_PWM_Stop(timer.htim, channel) != HAL_OK) { ErrorHandler("", 0); }

        SET_BIT(timer.tim->CCER, (uint32_t)(TIM_CCx_DISABLE << (channel_to_channelmul4[pin.channel] & 0x1FU)));

        // if timer supports break
        if constexpr ((dev.e.request == (TimerRequest)1) || (dev.e.request == (TimerRequest)8) || (dev.e.request == (TimerRequest)15) || (dev.e.request == (TimerRequest)16) || (dev.e.request == (TimerRequest)17))
        {
            // Disable Main Output Enable (MOE)
            CLEAR_BIT(timer.tim->BDTR, TIM_BDTR_MOE);
        }

        __HAL_TIM_DISABLE(timer.htim);

        HAL_TIM_ChannelStateTypeDef *state = &timer.htim.ChannelState[channel_to_channel_state_idx[pin.channel]];
        *state = HAL_TIM_CHANNEL_STATE_READY;

        is_on = false;
    }

    void set_duty_cycle(float duty_cycle) {
        uint16_t raw_duty = (uint16_t)((float)timer->tim->ARR / 200.0f * duty_cycle);
        //__HAL_TIM_SET_COMPARE(timer->htim, pin.channel, raw_duty);
        *(uint16_t*)((uint8_t*)(timer->tim) + channel_to_CCR[pin.channel]) = raw_duty;
        this->duty_cycle = duty_cycle;
    }

    void set_frequency(uint32_t frequency) {
        if(is_center_aligned) {
            frequency = 2*frequency;
        }
        this->frequency = frequency;
        timer.tim->ARR = (HAL_RCC_GetPCLK1Freq() * 2 / (timer.tim->PSC + 1)) / frequency;
        set_duty_cycle(duty_cycle);
    }

    void set_dead_time(int64_t dead_time_ns) {
    	TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

        int64_t time = dead_time_ns;

        if(time <= 127 * clock_period_ns) {
            sBreakDeadTimeConfig.DeadTime = time / clock_period_ns;
        } else if(time <= (2 * clock_period_ns * 127)) {
            sBreakDeadTimeConfig.DeadTime = time / (2 * clock_period_ns) - 64 + 128;
        } else if(time <= (8 * clock_period_ns * 127)) {
            sBreakDeadTimeConfig.DeadTime = time / (8 * clock_period_ns) -32 + 192;
        } else if(time <= (16 * clock_period_ns * 127)) {
            sBreakDeadTimeConfig.DeadTime = time / (16 * clock_period_ns) -32 + 224;
        } else {
            ErrorHandler("Invalid dead time configuration");
        }

        sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
        sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
        sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
        sBreakDeadTimeConfig.BreakState = TIM_BREAK_ENABLE;
        sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
        sBreakDeadTimeConfig.BreakFilter = 0;
        sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
        sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
        sBreakDeadTimeConfig.Break2Filter = 0;
        sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
        HAL_TIMEx_ConfigBreakDeadTime(timer->htim, &sBreakDeadTimeConfig);
        SET_BIT(timer->tim->BDTR, TIM_BDTR_MOE);
    }
};
} // namespace ST_LIB

#endif // HAL_TIM_MODULE_ENABLED
