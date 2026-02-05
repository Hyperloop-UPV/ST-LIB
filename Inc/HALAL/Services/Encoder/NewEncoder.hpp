#pragma once

#include "HALAL/Models/TimerDomain/TimerDomain.hpp"

#ifdef HAL_TIM_MODULE_ENABLED

namespace ST_LIB {

template<const TimerDomain::Timer &dev>
struct TimerWrapper;

template<const TimerDomain::Timer &dev, const ST_LIB::TimerPin pin1, const ST_LIB::TimerPin pin2>
class Encoder {
    static_assert(dev.e.pin_count == 2, "Encoder must have exactly 2 encoder pins, as it uses the whole timer");
    static_assert(dev.e.pins[0].af == TimerAF::Encoder, "Pin 0 must be declared as encoder");
    static_assert(dev.e.pins[1].af == TimerAF::Encoder, "Pin 1 must be declared as encoder");
    static_assert(dev.e.pins[0].channel != dev.e.pins[1].channel, "Pins must be of different channels");
    static_assert(pin1.channel != pin2.channel, "Encoder pins must be different channels (1 and 2)");
    

    inline static TimerWrapper<dev> *timer;
    inline static bool is_on = false;

public:
    Encoder(TimerWrapper<dev>* tim) {
         if (this->timer == nullptr) {
             init(tim);
         }
    }

    static void init(TimerWrapper<dev> *tim){
        timer = tim;
        TIM_Encoder_InitTypeDef sConfig = {0};
        TIM_MasterConfigTypeDef sMasterConfig = {0};

        tim->instance->hal_tim->Init.Prescaler = 5; 
        tim->instance->hal_tim->Init.CounterMode = TIM_COUNTERMODE_UP;
        tim->instance->hal_tim->Init.Period = 55000;
        tim->instance->hal_tim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
        tim->instance->hal_tim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

        sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
        sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
        sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
        sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
        sConfig.IC1Filter = 0;
        sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
        sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
        sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
        sConfig.IC2Filter = 0;

        if (HAL_TIM_Encoder_Init(tim->instance->hal_tim, &sConfig) != HAL_OK) {
            ErrorHandler("Unable to init encoder");
        }

        sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
        sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
        if (HAL_TIMEx_MasterConfigSynchronization(tim->instance->hal_tim, &sMasterConfig) != HAL_OK) {
            ErrorHandler("Unable to config master synchronization in encoder");
        }
    }

    static void turn_on();

    static void turn_off();

    static void reset();

    static uint32_t get_counter();

    static bool get_direction();

    static uint32_t get_initial_counter_value();

    static int64_t get_delta_clock(uint64_t clock_time, uint64_t last_clock_time);
};

}
#endif