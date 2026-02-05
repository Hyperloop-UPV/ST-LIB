#include "HALAL/Services/Encoder/NewEncoder.hpp"
#include "HALAL/Services/Time/TimerWrapper.hpp"

namespace ST_LIB {

#define NANO_SECOND 1000000000.0
#define CLOCK_MAX_VALUE 4294967295 //here goes the tim23 counter period


template<const TimerDomain::Timer &dev>
void Encoder<dev>::turn_on() {

    if (is_on) return;

    if(HAL_TIM_Encoder_GetState(timer->instance->hal_tim) == HAL_TIM_STATE_RESET) {
        ErrorHandler("Unable to get state from encoder");
        return;
    }
    if (HAL_TIM_Encoder_Start(timer->instance->hal_tim, TIM_CHANNEL_ALL) != HAL_OK) {
        ErrorHandler("Unable to start encoder");
        return;
    }
    is_on = true;
    reset();
    
}

template<const TimerDomain::Timer &dev>
void Encoder<dev>::turn_off() {
    if (!is_on) return;
        if (HAL_TIM_Encoder_Stop(timer->instance->hal_tim, TIM_CHANNEL_ALL) != HAL_OK) {
            ErrorHandler("Unable to stop encoder");
        }
        is_on = false;
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
