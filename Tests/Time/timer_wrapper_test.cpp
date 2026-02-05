#include <gtest/gtest.h>
//#include <thread>
//#include <chrono>

#include "HALAL/Models/TimerDomain/TimerDomain.hpp"
#include "HALAL/Services/Time/TimerWrapper.hpp"

TIM_TypeDef *ST_LIB::TimerDomain::cmsis_timers[16] = {
    [0] = TIM2_BASE,
    [1] = TIM3_BASE,
    [2] = TIM4_BASE,
    [3] = TIM5_BASE,
    [4] = TIM23_BASE,
    [5] = TIM24_BASE,
    [6] = TIM12_BASE,
    [7] = TIM13_BASE,
    [8] = TIM14_BASE,
    [9] = TIM15_BASE,
    [10] = TIM16_BASE,
    [11] = TIM17_BASE,
    [12] = TIM6_BASE,
    [13] = TIM7_BASE,
    [14] = TIM1_BASE,
    [15] = TIM8_BASE,
};

class TimerWrapperTests : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset Timer
        TIM1_BASE->CNT = 0;
        TIM1_BASE->ARR = 0;
        TIM1_BASE->SR = 0;
        TIM1_BASE->CR1 = 0;
        TIM1_BASE->DIER = 0;

    }
};

constexpr ST_LIB::TimerDomain::Timer tim1_decl{{
    .request = ST_LIB::TimerRequest::Advanced_1,
}};
ST_LIB::TimerDomain::Instance tim1_inst {
    .tim = TIM1_BASE,
    .hal_tim = 0,
    .timer_idx = 14,
};

TEST_F(TimerWrapperTests, Counter_EnabledDisabled) {
    ST_LIB::TimerWrapper<tim1_decl> tim1(&tim1_inst);

    tim1.counter_enable();
    EXPECT_EQ(TIM1_BASE->CR1 & TIM_CR1_CEN, TIM_CR1_CEN);
    tim1.counter_disable();
    EXPECT_EQ(TIM1_BASE->CR1 & TIM_CR1_CEN, 0);
}

TEST_F(TimerWrapperTests, UpdateInterrupt_EnabledDisabled) {
    ST_LIB::TimerWrapper<tim1_decl> tim1(&tim1_inst);

    tim1.enable_update_interrupt();
    EXPECT_EQ(TIM1_BASE->DIER & TIM_DIER_UIE, TIM_DIER_UIE);
    tim1.disable_update_interrupt();
    EXPECT_EQ(TIM1_BASE->DIER & TIM_DIER_UIE, 0);
}

TEST_F(TimerWrapperTests, NVIC_EnabledDisabled) {
    ST_LIB::TimerWrapper<tim1_decl> tim1(&tim1_inst);

    tim1.enable_nvic();
    EXPECT_EQ(NVIC_GetEnableIRQ(TIM1_UP_IRQn), 1);
    tim1.disable_nvic();
    EXPECT_EQ(NVIC_GetEnableIRQ(TIM1_UP_IRQn), 0);
}

TEST_F(TimerWrapperTests, UpdateEvent_EnabledDisabled) {
    ST_LIB::TimerWrapper<tim1_decl> tim1(&tim1_inst);

    tim1.enable_update_event();
    EXPECT_EQ(TIM1_BASE->CR1 & TIM_CR1_UDIS, 0);
    tim1.disable_update_event();
    EXPECT_EQ(TIM1_BASE->CR1 & TIM_CR1_UDIS, TIM_CR1_UDIS);
}

TEST_F(TimerWrapperTests, BreakInterrupt_EnabledDisabled) {
    ST_LIB::TimerWrapper<tim1_decl> tim1(&tim1_inst);

    tim1.break_interrupt_enable();
    EXPECT_EQ(TIM1_BASE->DIER & TIM_DIER_BIE, TIM_DIER_BIE);
    tim1.break_interrupt_disable();
    EXPECT_EQ(TIM1_BASE->DIER & TIM_DIER_BIE, 0);
}

TEST_F(TimerWrapperTests, OnePulseMode_EnabledDisabled) {
    ST_LIB::TimerWrapper<tim1_decl> tim1(&tim1_inst);

    tim1.set_one_pulse_mode();
    EXPECT_EQ(TIM1_BASE->CR1 & TIM_CR1_OPM, TIM_CR1_OPM);
    tim1.set_multi_interrupt_mode();
    EXPECT_EQ(TIM1_BASE->CR1 & TIM_CR1_OPM, 0);
}

void callback(void *raw) {}

TEST_F(TimerWrapperTests, ConfigureTimer) {
    ST_LIB::TimerWrapper<tim1_decl> tim1(&tim1_inst);

#define PRESCALER_VAL 200
#define PERIOD 1000
    tim1.set_prescaler(PRESCALER_VAL);
    tim1.configure16bit(callback, 0, PERIOD);
    EXPECT_EQ(static_cast<uint32_t>(TIM1_BASE->PSC), static_cast<uint32_t>(PRESCALER_VAL)); /* set prescaler */
    EXPECT_EQ(static_cast<uint32_t>(TIM1_BASE->ARR), static_cast<uint32_t>(PERIOD)); /* set period */
    EXPECT_EQ(TIM1_BASE->CR1 & TIM_CR1_CEN, TIM_CR1_CEN); /* set counter enable */
}

