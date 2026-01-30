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

template<const TimerDomain::Timer &dev, const ST_LIB::TimerPin pin>
class PWM {
    static consteval uint8_t get_channel_state_idx(const ST_LIB::TimerChannel ch) {
        switch(ch) {
            case TimerChannel::CHANNEL_1:
            case TimerChannel::CHANNEL_2:
            case TimerChannel::CHANNEL_3:
            case TimerChannel::CHANNEL_4:
            case TimerChannel::CHANNEL_5:
            case TimerChannel::CHANNEL_6:
                return static_cast<uint8_t>(ch) - 1;

            default: ST_LIB::compile_error("unreachable");
                return 0;
        }
    }

    static consteval uint8_t get_channel_mul4(const ST_LIB::TimerChannel ch) {
        switch(ch) {
            case TimerChannel::CHANNEL_1: return 0x00;
            case TimerChannel::CHANNEL_2: return 0x04;
            case TimerChannel::CHANNEL_3: return 0x08;
            case TimerChannel::CHANNEL_4: return 0x0C;
            case TimerChannel::CHANNEL_5: return 0x10;
            case TimerChannel::CHANNEL_6: return 0x14;

            default: ST_LIB::compile_error("unreachable");
                return 0;
        }
    }

    TimerWrapper<dev> *timer;
    uint32_t frequency;
    float duty_cycle;
    bool is_on = false;
    bool is_center_aligned;

public:
    PWM(TimerWrapper<dev> *tim, uint32_t polarity, uint32_t negated_polarity) : timer(tim) {
        this->is_center_aligned = ((timer->instance.tim->CR1 & TIM_CR1_CMS) != 0);

		TIM_OC_InitTypeDef sConfigOC = {
            .OCMode = TIM_OCMODE_PWM1,
            .Pulse = 0,

            .OCPolarity = polarity,
            .OCNPolarity = negated_polarity,
            
            .OCFastMode = TIM_OCFAST_DISABLE,
            .OCIdleState = TIM_OCIDLESTATE_RESET,
            .OCNIdleState = TIM_OCNIDLESTATE_RESET,
        };
        timer->template config_output_compare_channel<pin.channel>(&sConfigOC);
        timer->template set_output_compare_preload_enable<pin.channel>();
    }

    void turn_on() {
        if(this->is_on) return;
        
        //if(HAL_TIM_PWM_Start(timer->instance.hal_tim, channel) != HAL_OK) { ErrorHandler("", 0); }
        volatile HAL_TIM_ChannelStateTypeDef *state = &timer->instance.hal_tim->ChannelState[get_channel_state_idx(pin.channel)];
        if(*state != HAL_TIM_CHANNEL_STATE_READY) {
            ErrorHandler("Channel not ready");
        }

        *state = HAL_TIM_CHANNEL_STATE_BUSY;
        // enable CCx
        uint32_t enableCCx = TIM_CCER_CC1E << (get_channel_mul4(pin.channel) & 0x1FU); /* 0x1FU = 31 bits max shift */
        SET_BIT(timer->instance.tim->CCER, enableCCx);

        if constexpr(timer->is_break_instance) {
            // Main Output Enable
            SET_BIT(timer->instance.tim->BDTR, TIM_BDTR_MOE);
        }

        if constexpr(timer->is_slave_instance) {
            uint32_t tmpsmcr = timer->instance.tim->SMCR & TIM_SMCR_SMS;
            if(!IS_TIM_SLAVEMODE_TRIGGER_ENABLED(tmpsmcr)) {
                timer->counter_enable();
            }
        } else {
            timer->counter_enable();
        }

        if constexpr(pin.channel == ST_LIB::TimerChannel::CHANNEL_1) {
            SET_BIT(timer->instance.tim->CCER, TIM_CCER_CC1E);
        } else if constexpr(pin.channel == ST_LIB::TimerChannel::CHANNEL_2) {
            SET_BIT(timer->instance.tim->CCER, TIM_CCER_CC2E);
        } else if constexpr(pin.channel == ST_LIB::TimerChannel::CHANNEL_3) {
            SET_BIT(timer->instance.tim->CCER, TIM_CCER_CC3E);
        } else if constexpr(pin.channel == ST_LIB::TimerChannel::CHANNEL_4) {
            SET_BIT(timer->instance.tim->CCER, TIM_CCER_CC4E);
        } else if constexpr(pin.channel == ST_LIB::TimerChannel::CHANNEL_5) {
            SET_BIT(timer->instance.tim->CCER, TIM_CCER_CC5E);
        } else if constexpr(pin.channel == ST_LIB::TimerChannel::CHANNEL_6) {
            SET_BIT(timer->instance.tim->CCER, TIM_CCER_CC6E);
        }

        this->is_on = true;
    }

    void turn_off() {
        if(!this->is_on) return;
        // if(HAL_TIM_PWM_Stop(timer->instance.hal_tim, channel) != HAL_OK) { ErrorHandler("", 0); }

        SET_BIT(timer->tim->CCER, (uint32_t)(TIM_CCx_DISABLE << (get_channel_mul4(pin.channel) & 0x1FU)));

        if constexpr(timer->is_break_instance) {
            // Disable Main Output Enable (MOE)
            CLEAR_BIT(timer->tim->BDTR, TIM_BDTR_MOE);
        }

        HAL_TIM_ChannelStateTypeDef *state = &timer->instance.hal_tim.ChannelState[get_channel_state_idx(pin.channel)];
        *state = HAL_TIM_CHANNEL_STATE_READY;

        if(timer->instance.hal_tim->ChannelState[0] == HAL_TIM_CHANNEL_STATE_READY &&
           timer->instance.hal_tim->ChannelState[1] == HAL_TIM_CHANNEL_STATE_READY &&
           timer->instance.hal_tim->ChannelState[2] == HAL_TIM_CHANNEL_STATE_READY &&
           timer->instance.hal_tim->ChannelState[3] == HAL_TIM_CHANNEL_STATE_READY &&
           timer->instance.hal_tim->ChannelState[4] == HAL_TIM_CHANNEL_STATE_READY &&
           timer->instance.hal_tim->ChannelState[5] == HAL_TIM_CHANNEL_STATE_READY)
        {
            timer->counter_disable();
        }

        this->is_on = false;
    }

    void set_duty_cycle(float duty_cycle) {
        uint16_t raw_duty = (uint16_t)((float)(timer->instance.tim->ARR + 1) / 100.0f * duty_cycle);
        timer->template set_capture_compare<pin.channel>(raw_duty);
        this->duty_cycle = duty_cycle;
    }

    void set_frequency_quick(uint32_t frequency) {
        if(is_center_aligned) {
            frequency = 2*frequency;
        }
        this->frequency = frequency;
        timer->instance.tim->ARR = (timer->get_clock_frequency() / (timer->instance.tim->PSC + 1)) / frequency;

        set_duty_cycle(duty_cycle);
    }

    void set_frequency(uint32_t frequency) {
        if(is_center_aligned) {
            frequency = 2*frequency;
        }
        this->frequency = frequency;

        /* a = timer clock frequency 
         * b = (psc + 1) * frequency
         * arr = (a - b/2) / b
         */
        float psc_plus_1_mul_freq = (float)(timer->instance.tim->PSC + 1) * (float)frequency;
        timer->instance.tim->ARR = (uint32_t)((float)timer->get_clock_frequency() / psc_plus_1_mul_freq - 0.5f);

        set_duty_cycle(duty_cycle);
    }

    void configure(uint32_t frequency, float duty_cycle) {
        this->duty_cycle = duty_cycle;
        set_frequency(frequency);
    }

    void set_dead_time(int64_t dead_time_ns) {
    	TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

        int64_t time = dead_time_ns;

        float clock_period_ns = 1000.0f / (float)timer->get_clock_frequency();
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
        HAL_TIMEx_ConfigBreakDeadTime(timer->instance.hal_tim, &sBreakDeadTimeConfig);
        
        if constexpr(timer->is_break_instance) {
            // Main Output Enable
            SET_BIT(timer->tim->BDTR, TIM_BDTR_MOE);
        }
    }
};
} // namespace ST_LIB

#endif // HAL_TIM_MODULE_ENABLED
