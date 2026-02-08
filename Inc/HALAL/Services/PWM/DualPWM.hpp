/*
 * NewPWM.hpp
 *
 *  Created on: Jan 31, 2026
 *      Author: victor
 */
#pragma once

#include "HALAL/Models/TimerDomain/TimerDomain.hpp"
#ifdef HAL_TIM_MODULE_ENABLED
#include "HALAL/Models/GPIO.hpp"

namespace ST_LIB {

template<const TimerDomain::Timer &dev>
struct TimerWrapper;

template<const TimerDomain::Timer &dev, const ST_LIB::TimerPin pin, const ST_LIB::TimerPin negated_pin>
class DualPWM {
    static consteval uint8_t get_channel_state_idx(const ST_LIB::TimerChannel ch) {
        switch(ch) {
            case TimerChannel::CHANNEL_1: case TimerChannel::CHANNEL_1_NEGATED:
            case TimerChannel::CHANNEL_2: case TimerChannel::CHANNEL_2_NEGATED:
            case TimerChannel::CHANNEL_3: case TimerChannel::CHANNEL_3_NEGATED:
            case TimerChannel::CHANNEL_4:
            case TimerChannel::CHANNEL_5:
            case TimerChannel::CHANNEL_6:
                return (static_cast<uint8_t>(ch) & 
                        ~static_cast<uint8_t>(TimerChannel::CHANNEL_NEGATED_FLAG)) - 1;

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
    bool is_on_positive = false;
    bool is_on_negative = false;

public:
    DualPWM(TimerWrapper<dev> *tim, uint32_t polarity, uint32_t negated_polarity, float *duty_ptr, uint32_t *frequency_ptr) : timer(tim) {
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

    inline void turn_on() {
        turn_on_positive();
        turn_on_negative();
    }

    inline void turn_off() {
        turn_off_positive();
        turn_off_negative();
    }

    void turn_on_positive() {
        if(this->is_on_positive) return;
        
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

        this->is_on_positive = true;
    }

    void turn_on_negative() {
        if(this->is_on_negative) return;
        
        //if(HAL_TIM_PWM_Start(timer->instance->hal_tim, channel) != HAL_OK) { ErrorHandler("", 0); }
        volatile HAL_TIM_ChannelStateTypeDef *state = &timer->instance->hal_tim->ChannelNState[get_channel_state_idx(negated_pin.channel)];
        if(*state != HAL_TIM_CHANNEL_STATE_READY) {
            ErrorHandler("Channel not ready");
        }

        *state = HAL_TIM_CHANNEL_STATE_BUSY;
        // enable CCNx
        uint32_t enableCCNx = TIM_CCER_CC1NE << (get_channel_mul4(negated_pin.channel) & 0x1FU); /* 0x1FU = 31 bits max shift */
        SET_BIT(timer->instance->tim->CCER, enableCCNx);

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

        this->is_on_negative = true;
    }

    void turn_off_positive() {
        if(!this->is_on_positive) return;

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

        this->is_on_positive = false;
    }

    void turn_off_negative() {
        if(!this->is_on_negative) return;

        CLEAR_BIT(timer->tim->CCER, (uint32_t)(TIM_CCER_CC1NE << (get_channel_mul4(negated_pin.channel) & 0x1FU)));

        HAL_TIM_ChannelStateTypeDef *state = &timer->instance->hal_tim.ChannelState[get_channel_state_idx(negated_pin.channel)];
        *state = HAL_TIM_CHANNEL_STATE_READY;

        if(timer->are_all_channels_free()) {
            if constexpr(timer->is_break_instance) {
                // Disable Main Output Enable (MOE)
                CLEAR_BIT(timer->tim->BDTR, TIM_BDTR_MOE);
            }
            timer->counter_disable();
        }

        this->is_on_negative = false;
    }

    inline void set_duty_cycle(float duty_cycle) {
        if(duty_cycle <= 0.0f) [[unlikely]] {
            timer->template set_capture_compare<pin.channel>(0);
            *(this->duty_cycle) = 0.0f;
            return;
        }
        
        if(duty_cycle > 100.0f) [[unlikely]] { duty_cycle = 100.0f; }
        uint16_t raw_duty = (uint16_t)((float)(timer->instance->tim->ARR + 1) * (duty_cycle / 100.0f));
        timer->template set_capture_compare<pin.channel>(raw_duty);
        *(this->duty_cycle) = duty_cycle;
    }

    template<ST_LIB::PWM_Frequency_Mode mode = DEFAULT_PWM_FREQUENCY_MODE>
    inline void set_timer_frequency(uint32_t frequency) {
        timer->template set_pwm_frequency<mode>(frequency);
    }

    inline void configure(uint32_t frequency, float duty_cycle, int64_t dead_time_ns) {
        *(this->duty_cycle) = duty_cycle;
        this->template set_timer_frequency<DEFAULT_PWM_FREQUENCY_MODE>(frequency);
        this->set_dead_time(dead_time_ns);
    }

    inline uint32_t get_frequency() const {
        return *(this->frequency);
    }
    inline float get_duty_cycle() const {
        return *(this->duty_cycle);
    }

    void set_dead_time(int64_t dead_time_ns) {
    	TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

        int64_t time = dead_time_ns;

        float clock_period_ns = 1000'000'000.0f / (float)timer->get_clock_frequency();
        if(time <= 127 * clock_period_ns) {
            /* range 1: DTG[7] = 0; DTG[6:0] = [0..127]) */
            sBreakDeadTimeConfig.DeadTime = time / clock_period_ns;
        } else if(time <= (2 * clock_period_ns * 127)) {
            /* range 2: DTG[7:6] = 0b10; DTG[5:0] = 2*(64 + [0..63]) */
            sBreakDeadTimeConfig.DeadTime = 
                0b1000'0000 | (uint32_t)((float)time / (2 * clock_period_ns) - 64);
        } else if(time <= (8 * clock_period_ns * 63)) {
            /* range 3: DTG[7:5] = 0b110; DTG[4:0] = 8*(32 + [0..31]) */
            int64_t min = (8 * clock_period_ns * 32);
            if(time < min) { time = min; }
            sBreakDeadTimeConfig.DeadTime = 
                0b1100'0000 | (uint32_t)((float)time / (8 * clock_period_ns) - 32);
        } else if(time <= (16 * clock_period_ns * 63)) {
            /* range 4: DTG[7:5] = 0b111; DTG[4:0] = 16*(32 + [0..31]) */
            int64_t min = (16 * clock_period_ns * 32);
            if(time < min) { time = min; }
            sBreakDeadTimeConfig.DeadTime = 
                0b1110'0000 | (uint32_t)((float)time / (16 * clock_period_ns) - 32);
        } else {
            ErrorHandler("Invalid dead time configuration");
        }

        //sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
        //sBreakDeadTimeConfig.BreakState = TIM_BREAK_ENABLE;
        HAL_TIMEx_ConfigBreakDeadTime(timer->instance->hal_tim, &sBreakDeadTimeConfig);
        
        if constexpr(timer->is_break_instance) {
            // Main Output Enable
            SET_BIT(timer->instance->tim->BDTR, TIM_BDTR_MOE);
        }
    }
};
} // namespace ST_LIB

#endif // HAL_TIM_MODULE_ENABLED
