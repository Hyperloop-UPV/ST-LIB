#include "HALAL/Models/TimerDomain/TimerDomain.hpp"

using namespace ST_LIB;

#define X(n, b) TIM_HandleTypeDef htim##n;
TimerXList
#undef X

void TimerDomain::I_Need_To_Compile_TimerDomain_CPP(void) {
    // Need to compile this file so I keep this symbol here
}

void (*TimerDomain::callbacks[TimerDomain::max_instances])(void*) = {nullptr};
void *TimerDomain::callback_data[TimerDomain::max_instances] = {nullptr};

extern "C" void TIM1_IRQHandler(void) {
    CLEAR_BIT(TimerDomain::cmsis_timers[timer_idxmap[1]]->SR, TIM_SR_UIF);
    TimerDomain::callbacks[timer_idxmap[1]](TimerDomain::callback_data[timer_idxmap[1]]);
}

extern "C" void TIM2_IRQHandler(void) {
    CLEAR_BIT(TimerDomain::cmsis_timers[timer_idxmap[2]]->SR, TIM_SR_UIF);
    TimerDomain::callbacks[timer_idxmap[2]](TimerDomain::callback_data[timer_idxmap[2]]);
}

extern "C" void TIM3_IRQHandler(void) {
    CLEAR_BIT(TimerDomain::cmsis_timers[timer_idxmap[3]]->SR, TIM_SR_UIF);
    TimerDomain::callbacks[timer_idxmap[3]](TimerDomain::callback_data[timer_idxmap[3]]);
}

extern "C" void TIM4_IRQHandler(void) {
    CLEAR_BIT(TimerDomain::cmsis_timers[timer_idxmap[4]]->SR, TIM_SR_UIF);
    TimerDomain::callbacks[timer_idxmap[4]](TimerDomain::callback_data[timer_idxmap[4]]);
}

extern "C" void TIM5_IRQHandler(void) {
    CLEAR_BIT(TimerDomain::cmsis_timers[timer_idxmap[5]]->SR, TIM_SR_UIF);
    TimerDomain::callbacks[timer_idxmap[5]](TimerDomain::callback_data[timer_idxmap[5]]);
}

extern "C" void TIM6_IRQHandler(void) {
    CLEAR_BIT(TimerDomain::cmsis_timers[timer_idxmap[6]]->SR, TIM_SR_UIF);
    TimerDomain::callbacks[timer_idxmap[6]](TimerDomain::callback_data[timer_idxmap[6]]);
}

extern "C" void TIM7_IRQHandler(void) {
    CLEAR_BIT(TimerDomain::cmsis_timers[timer_idxmap[7]]->SR, TIM_SR_UIF);
    TimerDomain::callbacks[timer_idxmap[7]](TimerDomain::callback_data[timer_idxmap[7]]);
}

extern "C" void TIM8_IRQHandler(void) {
    CLEAR_BIT(TimerDomain::cmsis_timers[timer_idxmap[8]]->SR, TIM_SR_UIF);
    TimerDomain::callbacks[timer_idxmap[8]](TimerDomain::callback_data[timer_idxmap[8]]);
}



