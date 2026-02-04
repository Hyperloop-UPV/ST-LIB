/*
 * NewPWM.hpp
 *
 *  Created on: Dec 30, 2025
 *      Author: victor
 */
#pragma once

#include "HALAL/Models/TimerDomain/TimerDomain.hpp"
#ifdef HAL_TIM_MODULE_ENABLED
#include "HALAL/Models/GPIO.hpp"

namespace ST_LIB {

template<const TimerDomain::Timer &dev>
struct TimerWrapper;

template<const TimerDomain::Timer &dev, const ST_LIB::TimerPin pin>
class PWM {
    static consteval uint8_t get_channel_state_idx(const ST_LIB::TimerChannel ch) {
        switch(ch) {
            case TimerChannel::CHANNEL_1: case TimerChannel::CHANNEL_1_NEGATED:
            case TimerChannel::CHANNEL_2: case TimerChannel::CHANNEL_2_NEGATED:
            case TimerChannel::CHANNEL_3: case TimerChannel::CHANNEL_3_NEGATED:
            case TimerChannel::CHANNEL_4:
            case TimerChannel::CHANNEL_5:
            case TimerChannel::CHANNEL_6:
                return (static_cast<uint8_t>(ch) & 
                        static_cast<uint8_t>(TimerChannel::CHANNEL_NEGATED_FLAG)) - 1;

            default: ST_LIB::compile_error("unreachable");
                return 0;
        }
    }

    static consteval uint8_t get_channel_mul4(const ST_LIB::TimerChannel ch) {
        switch(ch) {
            case TimerChannel::CHANNEL_1: case TimerChannel::CHANNEL_1_NEGATED: return 0x00;
            case TimerChannel::CHANNEL_2: case TimerChannel::CHANNEL_2_NEGATED: return 0x04;
            case TimerChannel::CHANNEL_3: case TimerChannel::CHANNEL_3_NEGATED: return 0x08;
            case TimerChannel::CHANNEL_4:                                       return 0x0C;
            case TimerChannel::CHANNEL_5:                                       return 0x10;
            case TimerChannel::CHANNEL_6:                                       return 0x14;

            default: ST_LIB::compile_error("unreachable");
                return 0;
        }
    }

    TimerWrapper<dev> *timer;
    uint32_t *frequency;
    float *duty_cycle = nullptr;
    bool is_on = false;

public:
    PWM(TimerWrapper<dev> *tim, uint32_t polarity, uint32_t negated_polarity, float *duty_ptr, uint32_t *frequency_ptr) : timer(tim) {
        duty_cycle = duty_ptr;
        frequency = frequency_ptr;

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
        
        //if(HAL_TIM_PWM_Start(timer->instance->hal_tim, channel) != HAL_OK) { ErrorHandler("", 0); }
        volatile HAL_TIM_ChannelStateTypeDef *state = &timer->instance->hal_tim->ChannelState[get_channel_state_idx(pin.channel)];
        if(*state != HAL_TIM_CHANNEL_STATE_READY) {
            ErrorHandler("Channel not ready");
        }

        *state = HAL_TIM_CHANNEL_STATE_BUSY;
        // enable CCx
        uint32_t enableCCx = TIM_CCER_CC1E << (get_channel_mul4(pin.channel) & 0x1FU); /* 0x1FU = 31 bits max shift */
        SET_BIT(timer->instance->tim->CCER, enableCCx);

        if constexpr(timer->is_break_instance) {
            // Main Output Enable
            SET_BIT(timer->instance->tim->BDTR, TIM_BDTR_MOE);
        }

        if constexpr(timer->is_slave_instance) {
            uint32_t tmpsmcr = timer->instance->tim->SMCR & TIM_SMCR_SMS;
            if(!IS_TIM_SLAVEMODE_TRIGGER_ENABLED(tmpsmcr)) {
                timer->counter_enable();
            }
        } else {
            timer->counter_enable();
        }

        this->is_on = true;
    }

    void turn_off() {
        if(!this->is_on) return;

        CLEAR_BIT(timer->tim->CCER, (uint32_t)(TIM_CCER_CC1E << (get_channel_mul4(pin.channel) & 0x1FU)));

        HAL_TIM_ChannelStateTypeDef *state = &timer->instance->hal_tim.ChannelState[get_channel_state_idx(pin.channel)];
        *state = HAL_TIM_CHANNEL_STATE_READY;

        if(timer->are_all_channels_free()) {
            if constexpr(timer->is_break_instance) {
                // Disable Main Output Enable (MOE)
                CLEAR_BIT(timer->tim->BDTR, TIM_BDTR_MOE);
            }
            timer->counter_disable();
        }

        this->is_on = false;
    }

    void set_duty_cycle(float duty_cycle) {
        uint16_t raw_duty = (uint16_t)((float)(timer->instance->tim->ARR + 1) / (100.0f * duty_cycle));
        timer->template set_capture_compare<pin.channel>(raw_duty);
        *(this->duty_cycle) = duty_cycle;
    }

    template<ST_LIB::PWM_Frequency_Mode mode = DEFAULT_PWM_FREQUENCY_MODE>
    inline void set_timer_frequency(uint32_t frequency) {
        timer->template set_pwm_frequency<mode>(frequency);
    }

    inline void configure(uint32_t frequency, float duty_cycle) {
        *(this->duty_cycle) = duty_cycle;
        this->template set_timer_frequency<DEFAULT_PWM_FREQUENCY_MODE>(frequency);
    }

    inline uint32_t get_frequency() const {
        return *(this->frequency);
    }
    inline float get_duty_cycle() const {
        return *(this->duty_cycle);
    }
};
} // namespace ST_LIB

#endif // HAL_TIM_MODULE_ENABLED
