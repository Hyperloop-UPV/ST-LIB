#include "HALAL/Services/Encoder/NewEncoder.hpp"

namespace ST_LIB {

#define NANO_SECOND 1000000000.0
#define CLOCK_MAX_VALUE 4294967295 //here goes the tim23 counter period

template<const TimerDomain::Timer &dev>
void Encoder<dev>::init(TimerWrapper<dev> *tim, uint16_t prescaler, uint32_t period) {
    if constexpr(!tim->is_32bit_instance) {
        if(period > 0xFFFF) {
            ErrorHandler("Period too high for 16 bit timer");
            return;
        }
    }
    timer = tim;

    TIM_Encoder_InitTypeDef sConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    tim->instance->hal_tim->Init.Prescaler = prescaler;
    tim->instance->hal_tim->Init.CounterMode = TIM_COUNTERMODE_UP;
    tim->instance->hal_tim->Init.Period = period;
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

    if(HAL_TIM_Encoder_Init(timer->instance->hal_tim, &sConfig) != HAL_OK) {
        ErrorHandler("Unable to init encoder");
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if(HAL_TIMEx_MasterConfigSynchronization(timer->instance->hal_tim,
                                              &sMasterConfig) != HAL_OK) {
        ErrorHandler("Unable to config master synchronization in encoder");
    }
}

template<const TimerDomain::Timer &dev>
void Encoder<dev>::turn_on() {
    if(HAL_TIM_Encoder_GetState(timer->instance->hal_tim) == HAL_TIM_STATE_RESET) {
        ErrorHandler("Unable to get state from encoder");
        return;
    }

    if(HAL_TIM_Encoder_Start(timer->instance->hal_tim, TIM_CHANNEL_ALL) != HAL_OK) {
        ErrorHandler("Unable to start encoder");
        return;
    }

    reset();
}

template<const TimerDomain::Timer &dev>
void Encoder<dev>::turn_off() {
    if(HAL_TIM_Encoder_Stop(timer->instance->hal_tim, TIM_CHANNEL_ALL) != HAL_OK) {
        ErrorHandler("Unable to stop encoder");
    }
}

template<const TimerDomain::Timer &dev>
void Encoder<dev>::reset() {
    timer->instance->tim->CNT = UINT32_MAX / 2;
}

template<const TimerDomain::Timer &dev>
uint32_t Encoder<dev>::get_counter() {
    return timer->instance->tim->CNT;
}

template<const TimerDomain::Timer &dev>
bool Encoder<dev>::get_direction() {
    return ((timer->instance->tim->CR1 & 0b10000) >> 4);
}

template<const TimerDomain::Timer &dev>
uint32_t Encoder<dev>::get_initial_counter_value() {
    return timer->instance->tim->ARR / 2;
}

template<const TimerDomain::Timer &dev>
int64_t Encoder<dev>::get_delta_clock(uint64_t clock_time, uint64_t last_clock_time) {
    int64_t delta_clock = clock_time - last_clock_time;
    if (clock_time < last_clock_time) {  // overflow handle
        delta_clock =
            clock_time +
            CLOCK_MAX_VALUE * NANO_SECOND / timer->get_clock_frequency() -
            last_clock_time;
    }
    return delta_clock;
}


}
