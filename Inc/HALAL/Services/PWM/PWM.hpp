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

template <const TimerDomain::Timer& dev> struct TimerWrapper;

template <const TimerDomain::Timer& dev, const ST_LIB::TimerPin pin> class PWM {
    friend TimerWrapper<dev>;

    TimerWrapper<dev>* timer;
    uint32_t polarity;
    uint32_t negated_polarity;
    float* duty_cycle = nullptr;
    uint32_t* frequency;
    bool is_on = false;
    bool is_initialized = false;

    /* This constructor is private for a reason. Use TimerWrapper<dev>::get_pwm */
    PWM(TimerWrapper<dev>* tim,
        uint32_t polarity,
        uint32_t negated_polarity,
        float* duty_ptr,
        uint32_t* frequency_ptr)
        : timer(tim), polarity(polarity), negated_polarity(negated_polarity), duty_cycle(duty_ptr),
          frequency(frequency_ptr) {}
    void init() {
        if (is_initialized)
            return;
        if (timer == nullptr || timer->instance == nullptr || timer->instance->hal_tim == nullptr) {
            PANIC("Timer instance is not set for PWM");
            return;
        }
        TIM_OC_InitTypeDef sConfigOC = {
            .OCMode = TIM_OCMODE_PWM1,
            .Pulse = timer->template get_ccrx<pin.channel>(),
            .OCPolarity = polarity,
            .OCNPolarity = negated_polarity,
            .OCFastMode = TIM_OCFAST_DISABLE,
            .OCIdleState = TIM_OCIDLESTATE_RESET,
            .OCNIdleState = TIM_OCNIDLESTATE_RESET,
        };
        timer->template config_output_compare_channel<pin.channel>(&sConfigOC);
        timer->template set_output_compare_preload_enable<pin.channel>();
        is_initialized = true;
    }

public:
    void turn_on() {
        if (this->is_initialized == false) {
            init();
        }
        if (this->is_on)
            return;

        volatile HAL_TIM_ChannelStateTypeDef* state =
            &timer->instance->hal_tim
                 ->ChannelState[TimerDomain::get_channel_state_idx(pin.channel)];
        if (*state != HAL_TIM_CHANNEL_STATE_READY) {
            PANIC("Channel not ready");
        }

        *state = HAL_TIM_CHANNEL_STATE_BUSY;
        uint32_t enableCCx = TIM_CCER_CC1E
                             << (TimerDomain::get_channel_mul4(pin.channel) & 0x1FU
                                ); /* 0x1FU = 31 bits max shift */
        SET_BIT(timer->instance->tim->CCER, enableCCx);

        if constexpr (timer->is_break_instance) {
            // Main Output Enable
            SET_BIT(timer->instance->tim->BDTR, TIM_BDTR_MOE);
        }

        if constexpr (timer->is_slave_instance) {
            uint32_t tmpsmcr = timer->instance->tim->SMCR & TIM_SMCR_SMS;
            if (!IS_TIM_SLAVEMODE_TRIGGER_ENABLED(tmpsmcr)) {
                timer->counter_enable();
            }
        } else {
            timer->counter_enable();
        }

        this->is_on = true;
    }

    void turn_off() {
        if (!this->is_on)
            return;

        CLEAR_BIT(
            timer->instance->tim->CCER,
            (uint32_t)(TIM_CCER_CC1E << (TimerDomain::get_channel_mul4(pin.channel) & 0x1FU))
        );

        volatile HAL_TIM_ChannelStateTypeDef* state =
            &timer->instance->hal_tim
                 ->ChannelState[TimerDomain::get_channel_state_idx(pin.channel)];
        *state = HAL_TIM_CHANNEL_STATE_READY;

        if (timer->are_all_channels_free()) {
            if constexpr (timer->is_break_instance) {
                // Disable Main Output Enable (MOE)
                CLEAR_BIT(timer->instance->tim->BDTR, TIM_BDTR_MOE);
            }
            timer->counter_disable();
        }

        this->is_on = false;
    }

    inline void set_duty_cycle(float duty_cycle) {
        if (duty_cycle <= 0.0f) [[unlikely]] {
            timer->template set_capture_compare<pin.channel>(0);
            *(this->duty_cycle) = 0.0f;
            return;
        }

        if (duty_cycle > 100.0f) [[unlikely]] {
            duty_cycle = 100.0f;
        }
        if constexpr (timer->is_32bit_instance) {
            uint32_t raw_duty =
                (uint32_t)((float)(timer->instance->tim->ARR + 1) * (duty_cycle / 100.0f));
            timer->template set_capture_compare<pin.channel>(raw_duty);
        } else {
            uint16_t raw_duty =
                (uint16_t)((float)(timer->instance->tim->ARR + 1) * (duty_cycle / 100.0f));
            timer->template set_capture_compare<pin.channel>(raw_duty);
        }
        *(this->duty_cycle) = duty_cycle;
    }

    template <ST_LIB::PWM_Frequency_Mode mode = DEFAULT_PWM_FREQUENCY_MODE>
    inline void set_timer_frequency(uint32_t frequency) {
        timer->template set_pwm_frequency<mode>(frequency);
    }

    inline void configure(uint32_t frequency, float duty_cycle) {
        *(this->duty_cycle) = duty_cycle;
        this->template set_timer_frequency<DEFAULT_PWM_FREQUENCY_MODE>(frequency);
    }

    inline uint32_t get_frequency() const { return *(this->frequency); }
    inline float get_duty_cycle() const { return *(this->duty_cycle); }
};
} // namespace ST_LIB

#endif // HAL_TIM_MODULE_ENABLED
