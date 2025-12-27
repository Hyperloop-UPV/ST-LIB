#include "HALAL/Models/TimerDomain/TimerDomain.hpp"

template<TimerDomain::CounterMode mode>
TimerDomain::TimerWrapper<Device &dev>::set_mode() {
    constexpr uint8_t reqint = static_cast<uint8_t>(dev.e.request);
    if constexpr (!(reqint == 1 || reqint == 8 ||
        reqint == 2 || reqint == 5 || reqint == 23 || reqint == 24 || 
        reqint == 3 || reqint == 4))
    {
        compile_error("Error: In request reqidx: Timers other than {Advanced{TIM1, TIM8}, TIM2, TIM3, TIM4, TIM5, TIM23, TIM24} only support upcounting");
        return;
    }

    if constexpr (mode == CounterMode::UP) {
        MODIFY_REG(tim->CR1, TIM_CR1_CMS, 0);
        CLEAR_BIT(tim->CR1, TIM_CR1_DIR); // upcounter
    } else if constexpr (mode == CounterMode::DOWN) {
        MODIFY_REG(tim->CR1, TIM_CR1_CMS, 0);
        SET_BIT(tim->CR1, TIM_CR1_DIR); // downcounter
    } else if constexpr (mode == CounterMode::CENTER_ALIGNED_INTERRUPT_DOWN) {
        MODIFY_REG(tim->CR1, TIM_CR1_CMS, TIM_CR1_CMS_0);
    } else if constexpr (mode == CounterMode::CENTER_ALIGNED_INTERRUPT_UP) {
        MODIFY_REG(tim->CR1, TIM_CR1_CMS, TIM_CR1_CMS_1);
    } else if constexpr (mode == CounterMode::CENTER_ALIGNED_INTERRUPT_BOTH) {
        MODIFY_REG(tim->CR1, TIM_CR1_CMS, (TIM_CR1_CMS_0 | TIM_CR1_CMS_1));
    }
}

