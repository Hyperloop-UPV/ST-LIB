#include "HALAL/Models/TimerDomain/TimerDomain.hpp"

using namespace ST_LIB;

#define X(n, b) TIM_HandleTypeDef htim##n;
TimerXList
#undef X

void (*TimerDomain::callbacks[TimerDomain::max_instances])(void*) = {nullptr};
void *TimerDomain::callback_data[TimerDomain::max_instances] = {nullptr};

extern "C" void TIM1_UP_IRQHandler(void) {
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

extern "C" void TIM6_DAC_IRQHandler(void) {
    CLEAR_BIT(TimerDomain::cmsis_timers[timer_idxmap[6]]->SR, TIM_SR_UIF);
    TimerDomain::callbacks[timer_idxmap[6]](TimerDomain::callback_data[timer_idxmap[6]]);
}

extern "C" void TIM7_IRQHandler(void) {
    CLEAR_BIT(TimerDomain::cmsis_timers[timer_idxmap[7]]->SR, TIM_SR_UIF);
    TimerDomain::callbacks[timer_idxmap[7]](TimerDomain::callback_data[timer_idxmap[7]]);
}

/* NOTE: If it is needed, make changes so there's a specific callback for 
 *  tim8 which takes an int or something to know from which interrupt 
 *  it was called / make 3 callbacks
*/
extern "C" void TIM8_BRK_TIM12_IRQHandler(void) {
    CLEAR_BIT(TimerDomain::cmsis_timers[timer_idxmap[8]]->SR, TIM_SR_UIF);
    TimerDomain::callbacks[timer_idxmap[8]](TimerDomain::callback_data[timer_idxmap[8]]);
}
extern "C" void TIM8_UP_TIM13_IRQHandler(void) {
    CLEAR_BIT(TimerDomain::cmsis_timers[timer_idxmap[8]]->SR, TIM_SR_UIF);
    TimerDomain::callbacks[timer_idxmap[8]](TimerDomain::callback_data[timer_idxmap[8]]);
}
extern "C" void TIM8_TRG_COM_TIM14_IRQHandler(void) {
    CLEAR_BIT(TimerDomain::cmsis_timers[timer_idxmap[8]]->SR, TIM_SR_UIF);
    TimerDomain::callbacks[timer_idxmap[8]](TimerDomain::callback_data[timer_idxmap[8]]);
}

extern "C" void TIM15_IRQHandler(void) {
    CLEAR_BIT(TimerDomain::cmsis_timers[timer_idxmap[8]]->SR, TIM_SR_UIF);
    TimerDomain::callbacks[timer_idxmap[8]](TimerDomain::callback_data[timer_idxmap[8]]);
}

extern "C" void TIM16_IRQHandler(void) {
    CLEAR_BIT(TimerDomain::cmsis_timers[timer_idxmap[16]]->SR, TIM_SR_UIF);
    TimerDomain::callbacks[timer_idxmap[16]](TimerDomain::callback_data[timer_idxmap[16]]);
}

extern "C" void TIM17_IRQHandler(void) {
    CLEAR_BIT(TimerDomain::cmsis_timers[timer_idxmap[17]]->SR, TIM_SR_UIF);
    TimerDomain::callbacks[timer_idxmap[17]](TimerDomain::callback_data[timer_idxmap[17]]);
}

extern "C" void TIM23_IRQHandler(void) {
    CLEAR_BIT(TimerDomain::cmsis_timers[timer_idxmap[23]]->SR, TIM_SR_UIF);
    TimerDomain::callbacks[timer_idxmap[23]](TimerDomain::callback_data[timer_idxmap[23]]);
}

extern "C" void TIM24_IRQHandler(void) {
    CLEAR_BIT(TimerDomain::cmsis_timers[timer_idxmap[24]]->SR, TIM_SR_UIF);
    TimerDomain::callbacks[timer_idxmap[24]](TimerDomain::callback_data[timer_idxmap[24]]);
}
