/*
 * TimerWrapper.hpp
 *
 *  Created on: 30 dic. 2025
 *      Author: victor
 */

#pragma once

#include "HALAL/hal_wrapper.h"

#ifdef HAL_TIM_MODULE_ENABLED

#include "HALAL/Models/TimerDomain/TimerDomain.hpp"
#include "HALAL/Services/Encoder/NewEncoder.hpp"
#include "HALAL/Services/PWM/DualPWM.hpp"
#include "HALAL/Services/PWM/PWM.hpp"
#include "HALAL/Models/GPIO.hpp"

#include "ErrorHandler/ErrorHandler.hpp"

#define get_timer_instance(board, timer_type) \
    ST_LIB::TimerWrapper<timer_type>(&board::instance_of<timer_type>())

namespace ST_LIB {

template<const TimerDomain::Timer &dev>
struct TimerWrapper {
    TimerDomain::Instance *instance = nullptr;
    TimerWrapper() = default;
    TimerWrapper(TimerDomain::Instance *inst) : instance(inst) {}

    static constexpr int MAX_pwm_channel_duties = 4;
    float pwm_channel_duties[MAX_pwm_channel_duties] = {0.0f, 0.0f, 0.0f, 0.0f};
    uint32_t pwm_frequency = 0;

    enum CountingMode : uint8_t {
        UP = 0,
        DOWN = 1,
        /* center-aligned = counter counts up and down alternatively */
        /* Output compare interrupt flags of channels configured in output (CCxS=00 in TIMx_CCMRx register) are
           set only when the counter is counting down */
        CENTER_ALIGNED_INTERRUPT_DOWN = 2,
        /* Output compare interrupt flags of channels configured in output (CCxS=00 in TIMx_CCMRx register) are
           set only when the counter is counting up */
        CENTER_ALIGNED_INTERRUPT_UP = 3,
        /* both up and down */
        CENTER_ALIGNED_INTERRUPT_BOTH = 4,
    };

    static constexpr bool is_32bit_instance = (
        dev.e.request == TimerRequest::GeneralPurpose32bit_2 ||
        dev.e.request == TimerRequest::GeneralPurpose32bit_5 ||
        dev.e.request == TimerRequest::GeneralPurpose32bit_23 ||
        dev.e.request == TimerRequest::GeneralPurpose32bit_24 ||
        dev.e.request == TimerRequest::Any32bit
    );
    /* supports break input */
    static constexpr bool is_break_instance = (
        dev.e.request == TimerRequest::Advanced_1 || 
        dev.e.request == TimerRequest::Advanced_8 || 
        dev.e.request == TimerRequest::GeneralPurpose_15 || 
        dev.e.request == TimerRequest::GeneralPurpose_16 || 
        dev.e.request == TimerRequest::GeneralPurpose_17
    );
    static constexpr bool is_slave_instance = (
        dev.e.request == TimerRequest::Advanced_1 || 
        dev.e.request == TimerRequest::GeneralPurpose32bit_2 || 
        dev.e.request == TimerRequest::GeneralPurpose_3 || 
        dev.e.request == TimerRequest::GeneralPurpose_4 || 
        dev.e.request == TimerRequest::GeneralPurpose32bit_5 || 
        dev.e.request == TimerRequest::Advanced_8 || 
        dev.e.request == TimerRequest::SlaveTimer_12 || 
        dev.e.request == TimerRequest::GeneralPurpose_15 || 
        dev.e.request == TimerRequest::GeneralPurpose32bit_23 || 
        dev.e.request == TimerRequest::GeneralPurpose32bit_24
    );
    static constexpr bool only_supports_upcounting = (
        dev.e.request == TimerRequest::Basic_6 ||
        dev.e.request == TimerRequest::Basic_7 ||
        dev.e.request == TimerRequest::SlaveTimer_12 ||
        dev.e.request == TimerRequest::SlaveTimer_13 ||
        dev.e.request == TimerRequest::SlaveTimer_14 ||
        dev.e.request == TimerRequest::GeneralPurpose_15 ||
        dev.e.request == TimerRequest::GeneralPurpose_16 ||
        dev.e.request == TimerRequest::GeneralPurpose_17
    );
    static constexpr bool is_on_APB1 = (
        dev.e.request == TimerRequest::GeneralPurpose32bit_2 ||
        dev.e.request == TimerRequest::GeneralPurpose_3 ||
        dev.e.request == TimerRequest::GeneralPurpose_4 ||
        dev.e.request == TimerRequest::GeneralPurpose32bit_5 ||
        dev.e.request == TimerRequest::Basic_6 ||
        dev.e.request == TimerRequest::Basic_7 ||
        dev.e.request == TimerRequest::SlaveTimer_12 ||
        dev.e.request == TimerRequest::SlaveTimer_13 ||
        dev.e.request == TimerRequest::SlaveTimer_14
    );

    /* returns if the channel can be negated or not {See TimerDomain get_gpio_af()} */
    static consteval bool is_ccxn_instance(ST_LIB::TimerChannel ch) {
        switch(dev.e.request) {
            case TimerRequest::Advanced_1:
            case TimerRequest::Advanced_8:
                return (ch == TimerChannel::CHANNEL_1) || (ch == TimerChannel::CHANNEL_1_NEGATED) ||
                       (ch == TimerChannel::CHANNEL_2) || (ch == TimerChannel::CHANNEL_2_NEGATED) ||
                       (ch == TimerChannel::CHANNEL_3) || (ch == TimerChannel::CHANNEL_3_NEGATED);

            case TimerRequest::GeneralPurpose_15:
            case TimerRequest::GeneralPurpose_16:
            case TimerRequest::GeneralPurpose_17:
                return (ch == TimerChannel::CHANNEL_1) || (ch == TimerChannel::CHANNEL_1_NEGATED);
            
            default:
                return false;
        }
    }

    inline uint32_t get_clock_frequency() {
        uint32_t result;
        if constexpr(this->is_on_APB1) {
            result = HAL_RCC_GetPCLK1Freq();
            if((RCC->D2CFGR & RCC_D2CFGR_D2PPRE1) != RCC_HCLK_DIV1) {
                result *= 2;
            }
        } else {
            result = HAL_RCC_GetPCLK2Freq();
            if((RCC->D2CFGR & RCC_D2CFGR_D2PPRE2) != RCC_HCLK_DIV1) {
                result *= 2;
            }
        }
        return result;
    }

    template<TimerPin pin>
    inline PWM<dev, pin> get_pwm(uint32_t polarity = TIM_OCPOLARITY_HIGH, uint32_t negated_polarity = TIM_OCNPOLARITY_HIGH) {
        static_assert(dev.e.pin_count > 0, "Need at least one pin to get a pwm");
        
        if constexpr(dev.e.pins[0].pin == pin.pin && dev.e.pins[0].channel == pin.channel) {
            if constexpr(dev.e.pins[0].af != TimerAF::PWM) {
                ST_LIB::compile_error("Pin must be configured in TimerDomain as a PWM");
            }
            return PWM<dev, pin>(this, polarity, negated_polarity, 
                                 &pwm_channel_duties[(static_cast<uint8_t>(pin.channel) & 
                                    ~static_cast<uint8_t>(TimerChannel::CHANNEL_NEGATED_FLAG)) - 1], &pwm_frequency);
        } else if constexpr(dev.e.pin_count > 1 &&
            dev.e.pins[1].pin == pin.pin && dev.e.pins[1].channel == pin.channel)
        {
            if constexpr(dev.e.pins[1].af != TimerAF::PWM) {
                ST_LIB::compile_error("Pin must be configured in TimerDomain as a PWM");
            }
            return PWM<dev, pin>(this, polarity, negated_polarity, 
                                 &pwm_channel_duties[(static_cast<uint8_t>(pin.channel) & 
                                    ~static_cast<uint8_t>(TimerChannel::CHANNEL_NEGATED_FLAG)) - 1], &pwm_frequency);
        } else if constexpr(dev.e.pin_count > 2 &&
            dev.e.pins[2].pin == pin.pin && dev.e.pins[2].channel == pin.channel)
        {
            if constexpr(dev.e.pins[2].af != TimerAF::PWM) {
                ST_LIB::compile_error("Pin must be configured in TimerDomain as a PWM");
            }
            return PWM<dev, pin>(this, polarity, negated_polarity, 
                                 &pwm_channel_duties[(static_cast<uint8_t>(pin.channel) & 
                                    ~static_cast<uint8_t>(TimerChannel::CHANNEL_NEGATED_FLAG)) - 1], &pwm_frequency);
        } else if constexpr(dev.e.pin_count > 3 &&
            dev.e.pins[3].pin == pin.pin && dev.e.pins[3].channel == pin.channel)
        {
            if constexpr(dev.e.pins[3].af != TimerAF::PWM) {
                ST_LIB::compile_error("Pin must be configured in TimerDomain as a PWM");
            }
            return PWM<dev, pin>(this, polarity, negated_polarity, 
                                 &pwm_channel_duties[(static_cast<uint8_t>(pin.channel) & 
                                    ~static_cast<uint8_t>(TimerChannel::CHANNEL_NEGATED_FLAG)) - 1], &pwm_frequency);
        } else if constexpr(dev.e.pin_count > 4 &&
            dev.e.pins[4].pin == pin.pin && dev.e.pins[4].channel == pin.channel)
        {
            if constexpr(dev.e.pins[4].af != TimerAF::PWM) {
                ST_LIB::compile_error("Pin must be configured in TimerDomain as a PWM");
            }
            return PWM<dev, pin>(this, polarity, negated_polarity, 
                                 &pwm_channel_duties[(static_cast<uint8_t>(pin.channel) & 
                                    ~static_cast<uint8_t>(TimerChannel::CHANNEL_NEGATED_FLAG)) - 1], &pwm_frequency);
        } else if constexpr(dev.e.pin_count > 5 &&
            dev.e.pins[5].pin == pin.pin && dev.e.pins[5].channel == pin.channel)
        {
            if constexpr(dev.e.pins[5].af != TimerAF::PWM) {
                ST_LIB::compile_error("Pin must be configured in TimerDomain as a PWM");
            }
            return PWM<dev, pin>(this, polarity, negated_polarity, 
                                 &pwm_channel_duties[(static_cast<uint8_t>(pin.channel) & 
                                    ~static_cast<uint8_t>(TimerChannel::CHANNEL_NEGATED_FLAG)) - 1], &pwm_frequency);
        } else if constexpr(dev.e.pin_count > 6 &&
            dev.e.pins[6].pin == pin.pin && dev.e.pins[6].channel == pin.channel)
        {
            if constexpr(dev.e.pins[6].af != TimerAF::PWM) {
                ST_LIB::compile_error("Pin must be configured in TimerDomain as a PWM");
            }
            return PWM<dev, pin>(this, polarity, negated_polarity, 
                                 &pwm_channel_duties[(static_cast<uint8_t>(pin.channel) & 
                                    ~static_cast<uint8_t>(TimerChannel::CHANNEL_NEGATED_FLAG)) - 1], &pwm_frequency);
        } else if constexpr(dev.e.pin_count == 7 &&
            dev.e.pins[7].pin == pin.pin && dev.e.pins[7].channel == pin.channel)
        {
            if constexpr(dev.e.pins[7].af != TimerAF::PWM) {
                ST_LIB::compile_error("Pin must be configured in TimerDomain as a PWM");
            }
            return PWM<dev, pin>(this, polarity, negated_polarity, 
                                 &pwm_channel_duties[(static_cast<uint8_t>(pin.channel) & 
                                    ~static_cast<uint8_t>(TimerChannel::CHANNEL_NEGATED_FLAG)) - 1], &pwm_frequency);
        } else {
            ST_LIB::compile_error("No pins passed to TimerWrapper are the same as the pins passed to get_pwm() [this method]");
        }
    }

    template<TimerPin pin, TimerPin negated_pin>
    inline DualPWM<dev, pin, negated_pin> get_dual_pwm(uint32_t polarity = TIM_OCPOLARITY_HIGH, uint32_t negated_polarity = TIM_OCNPOLARITY_HIGH)
    {
        static_assert(dev.e.pin_count > 1, "Need at least two pins on one timer to get a dual pwm");

        // NOTE: No voy a comprobar todo esto, no hará falta comprobarlo con la siguiente versión del pwm, dualpwm
        static_assert((static_cast<uint8_t>(pin.channel) | static_cast<uint8_t>(TimerChannel::CHANNEL_NEGATED_FLAG)) == static_cast<uint8_t>(negated_pin.channel));
        static_assert(pin.af == TimerAF::PWM && negated_pin.af == TimerAF::PWM);
        return DualPWM<dev, pin, negated_pin>(this, polarity, negated_polarity, 
            &pwm_channel_duties[static_cast<uint8_t>(pin.channel) - 1], &pwm_frequency);
    }

    inline Encoder<dev> get_encoder()
    {        
        return Encoder<dev>(this);
    }

    inline void counter_enable() {
        SET_BIT(instance->tim->CR1, TIM_CR1_CEN);
    }
    inline void counter_disable() {
        CLEAR_BIT(instance->tim->CR1, TIM_CR1_CEN);
    }

    inline void clear_update_interrupt_flag() {
        CLEAR_BIT(instance->tim->SR, TIM_SR_UIF);
    }

    /* Disabled by default */
    inline void enable_update_interrupt() {
        SET_BIT(instance->tim->DIER, TIM_DIER_UIE);
    }
    inline void disable_update_interrupt() {
        CLEAR_BIT(instance->tim->DIER, TIM_DIER_UIE);
    }

    /* Disabled by default */
    inline void enable_nvic() {
        NVIC_EnableIRQ(TimerDomain::timer_irqn[instance->timer_idx]);
    }
    inline void disable_nvic() {
        NVIC_DisableIRQ(TimerDomain::timer_irqn[instance->timer_idx]);
    }

    /* Enable UEV. The Update (UEV) event is generated by one of the following events:
     * – Counter overflow/underflow
     * – Setting the UG bit
     * – Update generation through the slave mode controller    
     * Enabled by default
     */
    inline void enable_update_event() {
        CLEAR_BIT(instance->tim->CR1, TIM_CR1_UDIS);
    }
    /* Disable UEV. The Update event is not generated, shadow registers keep their value
     * (ARR, PSC, CCRx). However the counter and the prescaler are reinitialized if the UG bit
     * is set or if a hardware reset is received from the slave mode controller.
     */
    inline void disable_update_event() {
        SET_BIT(instance->tim->CR1, TIM_CR1_UDIS);
    }

    /* default: disabled */
    inline void break_interrupt_enable() {
        static_assert(dev.e.request == TimerRequest::Advanced_1 || dev.e.request == TimerRequest::Advanced_8,
            "Error: Break interrupt enable only allowed in advanced timers {TIM1, TIM8}");
        SET_BIT(instance->tim->DIER, TIM_DIER_BIE);
    }
    inline void break_interrupt_disable() {
        static_assert(dev.e.request == TimerRequest::Advanced_1 || dev.e.request == TimerRequest::Advanced_8,
            "Error: Break interrupt enable only allowed in advanced timers {TIM1, TIM8}");
        CLEAR_BIT(instance->tim->DIER, TIM_DIER_BIE);
    }

    /* interrupt gets called only once, counter needs to be reenabled */
    inline void set_one_pulse_mode() {
        SET_BIT(instance->tim->CR1, TIM_CR1_OPM);
    }
    inline void set_multi_interrupt_mode() {
        CLEAR_BIT(instance->tim->CR1, TIM_CR1_OPM);
    }

    inline TIM_HandleTypeDef *get_hal_handle() {
        return instance->hal_tim;
    }
    inline TIM_TypeDef *get_cmsis_handle() {
        return instance->tim;
    }

    inline void set_prescaler(uint16_t psc) {
        instance->tim->PSC = psc;
    }

    inline void configure32bit(void (*callback)(void*), void *callback_data, uint32_t period)
    {
        static_assert(this->is_32bit_instance, "Only timers {TIM2, TIM5, TIM23, TIM24} have a 32-bit resolution");

        instance->tim->ARR = period;
        TimerDomain::callbacks[instance->timer_idx] = callback;
        TimerDomain::callback_data[instance->timer_idx] = callback_data;
        this->counter_enable();
    }

    inline void configure16bit(void (*callback)(void*), void *callback_data, uint16_t period)
    {
        instance->tim->ARR = period;
        TimerDomain::callbacks[instance->timer_idx] = callback;
        TimerDomain::callback_data[instance->timer_idx] = callback_data;
        this->counter_enable();
    }

    /* WARNING: The counter _must_ be disabled to switch from edge-aligned to center-aligned mode */
    template<CountingMode mode>
    inline void set_counting_mode(void) {
        constexpr uint8_t reqint = static_cast<uint8_t>(dev.e.request);
        static_assert(!this->only_supports_upcounting,
            "Error: In request reqidx: Timers other than {Advanced{TIM1, TIM8}, TIM2, TIM3, TIM4, TIM5, TIM23, TIM24} only support upcounting");

        if constexpr (mode == CountingMode::UP) {
            MODIFY_REG(instance->tim->CR1, TIM_CR1_CMS, 0);
            CLEAR_BIT(instance->tim->CR1, TIM_CR1_DIR); // upcounter
        } else if constexpr (mode == CountingMode::DOWN) {
            MODIFY_REG(instance->tim->CR1, TIM_CR1_CMS, 0);
            SET_BIT(instance->tim->CR1, TIM_CR1_DIR); // downcounter
        } else if constexpr (mode == CountingMode::CENTER_ALIGNED_INTERRUPT_DOWN) {
            MODIFY_REG(instance->tim->CR1, TIM_CR1_CMS, TIM_CR1_CMS_0);
        } else if constexpr (mode == CountingMode::CENTER_ALIGNED_INTERRUPT_UP) {
            MODIFY_REG(instance->tim->CR1, TIM_CR1_CMS, TIM_CR1_CMS_1);
        } else if constexpr (mode == CountingMode::CENTER_ALIGNED_INTERRUPT_BOTH) {
            MODIFY_REG(instance->tim->CR1, TIM_CR1_CMS, (TIM_CR1_CMS_0 | TIM_CR1_CMS_1));
        }
    }

    ///////////////////////////////////////////
    // Below are methods used by other objects

    template<ST_LIB::PWM_Frequency_Mode mode = DEFAULT_PWM_FREQUENCY_MODE>
    void set_pwm_frequency(uint32_t frequency) {
        /* If it's center aligned duplicate the frequency */
        if((instance->tim->CR1 & TIM_CR1_CMS) != 0) {
            frequency = 2*frequency;
        }
        pwm_frequency = frequency;

        if constexpr(mode == ST_LIB::PWM_Frequency_Mode::SPEED) {
            instance->tim->ARR = (this->get_clock_frequency() / (instance->tim->PSC + 1)) / frequency;
        } else if constexpr(mode == ST_LIB::PWM_Frequency_Mode::PRECISION) {
            float psc_plus_1_mul_freq = (float)(instance->tim->PSC + 1) * (float)frequency;
            instance->tim->ARR = (uint32_t)((float)get_clock_frequency() / psc_plus_1_mul_freq - 0.5f);
        }

        if(pwm_channel_duties[0] != 0.0f) {
            uint16_t raw_duty = (uint16_t)((float)(instance->tim->ARR + 1) / 100.0f * pwm_channel_duties[0]);
            instance->tim->CCR1 = raw_duty;
        }
        if(pwm_channel_duties[1] != 0.0f) {
            uint16_t raw_duty = (uint16_t)((float)(instance->tim->ARR + 1) / 100.0f * pwm_channel_duties[1]);
            instance->tim->CCR2 = raw_duty;
        }
        if(pwm_channel_duties[2] != 0.0f) {
            uint16_t raw_duty = (uint16_t)((float)(instance->tim->ARR + 1) / 100.0f * pwm_channel_duties[2]);
            instance->tim->CCR3 = raw_duty;
        }
        if(pwm_channel_duties[3] != 0.0f) {
            uint16_t raw_duty = (uint16_t)((float)(instance->tim->ARR + 1) / 100.0f * pwm_channel_duties[3]);
            instance->tim->CCR4 = raw_duty;
        }
    }

    template<ST_LIB::TimerChannel ch>
    inline void config_output_compare_channel(const TIM_OC_InitTypeDef *OC_Config)
    {
        if constexpr(!((ch == TimerChannel::CHANNEL_1) || (ch == TimerChannel::CHANNEL_2) ||
                       (ch == TimerChannel::CHANNEL_3) || (ch == TimerChannel::CHANNEL_4) ||
                       (ch == TimerChannel::CHANNEL_5) || (ch == TimerChannel::CHANNEL_6)))
        {
            ST_LIB::compile_error("config_output_compare_channel should only be called with channels [1..6] (not negated)");
        }

        uint32_t tmpccmrx;
        uint32_t tmpccer;
        uint32_t tmpcr2;

        tmpccer = instance->tim->CCER;

        tmpcr2 = instance->tim->CR2;
        
        if constexpr(ch == TimerChannel::CHANNEL_1 || ch == TimerChannel::CHANNEL_2) {
            tmpccmrx = instance->tim->CCMR1;
        } else if constexpr(ch == TimerChannel::CHANNEL_3 || ch == TimerChannel::CHANNEL_4) {
            tmpccmrx = instance->tim->CCMR2;
        } else if constexpr(ch == TimerChannel::CHANNEL_5 || ch == TimerChannel::CHANNEL_6) {
            tmpccmrx = instance->tim->CCMR3;
        }

        if constexpr(ch == TimerChannel::CHANNEL_1) {
            CLEAR_BIT(instance->tim->CCER, TIM_CCER_CC1E);

            CLEAR_BIT(tmpccmrx, TIM_CCMR1_OC1M);
            CLEAR_BIT(tmpccmrx, TIM_CCMR1_CC1S);

            SET_BIT(tmpccmrx, OC_Config->OCMode);
            CLEAR_BIT(tmpccer, TIM_CCER_CC1P);
            SET_BIT(tmpccer, OC_Config->OCPolarity);
        } else if constexpr(ch == TimerChannel::CHANNEL_2) {
            CLEAR_BIT(instance->tim->CCER, TIM_CCER_CC2E);

            CLEAR_BIT(tmpccmrx, TIM_CCMR1_OC2M);
            CLEAR_BIT(tmpccmrx, TIM_CCMR1_CC2S);

            SET_BIT(tmpccmrx, OC_Config->OCMode << 8U);
            CLEAR_BIT(tmpccer, TIM_CCER_CC2P);
            SET_BIT(tmpccer, OC_Config->OCPolarity << 4U);
        } else if constexpr(ch == TimerChannel::CHANNEL_3) {
            CLEAR_BIT(instance->tim->CCER, TIM_CCER_CC3E);

            CLEAR_BIT(tmpccmrx, TIM_CCMR2_OC3M);
            CLEAR_BIT(tmpccmrx, TIM_CCMR2_CC3S);

            SET_BIT(tmpccmrx, OC_Config->OCMode);
            CLEAR_BIT(tmpccer, TIM_CCER_CC3P);
            SET_BIT(tmpccer, OC_Config->OCPolarity << 8U);
        } else if constexpr(ch == TimerChannel::CHANNEL_4) {
            CLEAR_BIT(instance->tim->CCER, TIM_CCER_CC4E);

            CLEAR_BIT(tmpccmrx, TIM_CCMR2_OC4M);
            CLEAR_BIT(tmpccmrx, TIM_CCMR2_CC4S);

            SET_BIT(tmpccmrx, OC_Config->OCMode);
            CLEAR_BIT(tmpccer, TIM_CCER_CC4P);
            SET_BIT(tmpccer, OC_Config->OCPolarity << 12U);
        } else if constexpr(ch == TimerChannel::CHANNEL_5) {
            CLEAR_BIT(instance->tim->CCER, TIM_CCER_CC5E);

            CLEAR_BIT(tmpccmrx, TIM_CCMR3_OC5M);

            SET_BIT(tmpccmrx, OC_Config->OCMode);
            CLEAR_BIT(tmpccer, TIM_CCER_CC5P);
            SET_BIT(tmpccer, OC_Config->OCPolarity << 16U);
        } else if constexpr(ch == TimerChannel::CHANNEL_6) {
            CLEAR_BIT(instance->tim->CCER, TIM_CCER_CC6E);

            CLEAR_BIT(tmpccmrx, TIM_CCMR3_OC6M);

            SET_BIT(tmpccmrx, OC_Config->OCMode);
            CLEAR_BIT(tmpccer, TIM_CCER_CC6P);
            SET_BIT(tmpccer, OC_Config->OCPolarity << 20U);
        }

        if(this->is_ccxn_instance(ch)) {
            assert_param(IS_TIM_OCN_POLARITY(OC_Config->OCNPolarity));

            if constexpr(ch == TimerChannel::CHANNEL_1) {
                CLEAR_BIT(tmpccer, TIM_CCER_CC1NP);
                SET_BIT(tmpccer, OC_Config->OCNPolarity);
                CLEAR_BIT(tmpccer, TIM_CCER_CC1NE);
            } else if constexpr(ch == TimerChannel::CHANNEL_2) {
                CLEAR_BIT(tmpccer, TIM_CCER_CC2NP);
                SET_BIT(tmpccer, OC_Config->OCNPolarity << 4U);
                CLEAR_BIT(tmpccer, TIM_CCER_CC2NE);
            } else if constexpr(ch == TimerChannel::CHANNEL_3) {
                CLEAR_BIT(tmpccer, TIM_CCER_CC3NP);
                SET_BIT(tmpccer, OC_Config->OCNPolarity << 8U);
                CLEAR_BIT(tmpccer, TIM_CCER_CC3NE);
            }
        }

        if(this->is_break_instance) {
            assert_param(IS_TIM_OCNIDLE_STATE(OC_Config->OCNIdleState));
            assert_param(IS_TIM_OCIDLE_STATE(OC_Config->OCIdleState));

            if constexpr(ch == TimerChannel::CHANNEL_1) {
                CLEAR_BIT(tmpcr2, TIM_CR2_OIS1);
                CLEAR_BIT(tmpcr2, TIM_CR2_OIS1N);
                SET_BIT(tmpcr2, OC_Config->OCNIdleState);
                SET_BIT(tmpcr2, OC_Config->OCIdleState);
            } else if constexpr(ch == TimerChannel::CHANNEL_2) {
                CLEAR_BIT(tmpcr2, TIM_CR2_OIS2);
                CLEAR_BIT(tmpcr2, TIM_CR2_OIS2N);
                SET_BIT(tmpcr2, OC_Config->OCNIdleState << 2U);
                SET_BIT(tmpcr2, OC_Config->OCIdleState << 2U);
            } else if constexpr(ch == TimerChannel::CHANNEL_3) {
                CLEAR_BIT(tmpcr2, TIM_CR2_OIS3);
                CLEAR_BIT(tmpcr2, TIM_CR2_OIS3N);
                SET_BIT(tmpcr2, OC_Config->OCNIdleState << 4U);
                SET_BIT(tmpcr2, OC_Config->OCIdleState << 4U);
            } else if constexpr(ch == TimerChannel::CHANNEL_4) {
                CLEAR_BIT(tmpcr2, TIM_CR2_OIS4);
                SET_BIT(tmpcr2, OC_Config->OCIdleState << 6U);
            } else if constexpr(ch == TimerChannel::CHANNEL_5) {
                CLEAR_BIT(tmpcr2, TIM_CR2_OIS5);
                SET_BIT(tmpcr2, OC_Config->OCIdleState << 8U);
            } else if constexpr(ch == TimerChannel::CHANNEL_6) {
                CLEAR_BIT(tmpcr2, TIM_CR2_OIS6);
                SET_BIT(tmpcr2, OC_Config->OCIdleState << 10U);
            }
        }

        instance->tim->CR2 = tmpcr2;
        if constexpr(ch == TimerChannel::CHANNEL_1 || ch == TimerChannel::CHANNEL_2) {
            instance->tim->CCMR1 = tmpccmrx;
        } else if constexpr(ch == TimerChannel::CHANNEL_3 || ch == TimerChannel::CHANNEL_4) {
            instance->tim->CCMR2 = tmpccmrx;
        } else if constexpr(ch == TimerChannel::CHANNEL_5 || ch == TimerChannel::CHANNEL_6) {
            instance->tim->CCMR3 = tmpccmrx;
        }

        if constexpr(ch == TimerChannel::CHANNEL_1)      instance->tim->CCR1 = OC_Config->Pulse;
        else if constexpr(ch == TimerChannel::CHANNEL_2) instance->tim->CCR2 = OC_Config->Pulse;
        else if constexpr(ch == TimerChannel::CHANNEL_3) instance->tim->CCR3 = OC_Config->Pulse;
        else if constexpr(ch == TimerChannel::CHANNEL_4) instance->tim->CCR4 = OC_Config->Pulse;
        else if constexpr(ch == TimerChannel::CHANNEL_5) instance->tim->CCR5 = OC_Config->Pulse;
        else if constexpr(ch == TimerChannel::CHANNEL_6) instance->tim->CCR6 = OC_Config->Pulse;

        instance->tim->CCER = tmpccer;
    }

    template<ST_LIB::TimerChannel ch>
    inline void set_output_compare_preload_enable()
    {
        if constexpr((ch == TimerChannel::CHANNEL_1) || (ch == TimerChannel::CHANNEL_1_NEGATED)) {
            SET_BIT(instance->tim->CCMR1, TIM_CCMR1_OC1PE);
        } else if constexpr((ch == TimerChannel::CHANNEL_2) || (ch == TimerChannel::CHANNEL_2_NEGATED)) {
            SET_BIT(instance->tim->CCMR1, TIM_CCMR1_OC2PE);
        } else if constexpr((ch == TimerChannel::CHANNEL_3) || (ch == TimerChannel::CHANNEL_3_NEGATED)) {
            SET_BIT(instance->tim->CCMR2, TIM_CCMR2_OC3PE);
        } else if constexpr(ch == TimerChannel::CHANNEL_4) {
            SET_BIT(instance->tim->CCMR2, TIM_CCMR2_OC4PE);
        } else if constexpr(ch == TimerChannel::CHANNEL_5) {
            SET_BIT(instance->tim->CCMR3, TIM_CCMR3_OC5PE);
        } else if constexpr(ch == TimerChannel::CHANNEL_6) {
            SET_BIT(instance->tim->CCMR3, TIM_CCMR3_OC6PE);
        } else ST_LIB::compile_error("Unknown timer channel, there are only 6 channels [1..6]");
    }

    template<ST_LIB::TimerChannel ch>
    inline void set_capture_compare(uint16_t val) {
        if constexpr((ch == TimerChannel::CHANNEL_1) || (ch == TimerChannel::CHANNEL_1_NEGATED)) {
            instance->tim->CCR1 = val;
        } else if constexpr((ch == TimerChannel::CHANNEL_2) || (ch == TimerChannel::CHANNEL_2_NEGATED)) {
            instance->tim->CCR2 = val;
        } else if constexpr((ch == TimerChannel::CHANNEL_3) || (ch == TimerChannel::CHANNEL_3_NEGATED)) {
            instance->tim->CCR3 = val;
        } else if constexpr(ch == TimerChannel::CHANNEL_4) {
            instance->tim->CCR4 = val;
        } else if constexpr(ch == TimerChannel::CHANNEL_5) {
            instance->tim->CCR5 = val;
        } else if constexpr(ch == TimerChannel::CHANNEL_6) {
            instance->tim->CCR6 = val;
        } else ST_LIB::compile_error("Unknown timer channel, there are only 6 channels [1..6]");
    }

    inline bool are_all_channels_free() {
        return ((instance->tim->CCER & TIM_CCER_CCxE_MASK) == 0) &&
               ((instance->tim->CCER & TIM_CCER_CCxNE_MASK) == 0);
    }

    // leftover from old TimerPeripheral, maybe this was useful?
    inline uint16_t get_prescaler() {
        return instance->tim->PSC;
    }
    inline uint32_t get_period() {
        return instance->tim->ARR;
    }
};
} // namespace ST_LIB

#endif // HAL_TIM_MODULE_ENABLED
