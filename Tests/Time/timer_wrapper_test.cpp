#include <gtest/gtest.h>
#include <thread>
#include <chrono>

#include "HALAL/Services/Timer/TimerWrapper.hpp"
#include "ST-LIB.hpp"

class TimerWrapperTests : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset global callback task count
        count = 0;
        
        // Reset Timer
        TIM1_BASE->CNT = 0;
        TIM1_BASE->ARR = 0;
        TIM1_BASE->SR = 0;
        TIM1_BASE->CR1 = 0;
        TIM1_BASE->DIER = 0;
    }
};

constexpr ST_LIB::TimerDomain::Timer tim1{{
    .request = ST_LIB::TimerRequest::Advanced_1,
}};

TEST_F(TimerWrapperTests, Counter_EnabledDisabled) {
    auto testBoard = ST_LIB::Board<tim2>;
    testBoard.init();

    auto tim2 = get_timer_instance(testBoard, tim2);
    tim2.counter_enable();
    EXPECT_EQ(TIM1_BASE->CR1 & TIM_CR1_CEN, TIM_CR1_CEN);
    tim2.counter_disable();
    EXPECT_EQ(TIM1_BASE->CR1 & TIM_CR1_CEN, 0);
}

TEST_F(TimerWrapperTests, UpdateInterrupt_EnabledDisabled) {
    auto testBoard = ST_LIB::Board<tim2>;
    testBoard.init();

    auto tim2 = get_timer_instance(testBoard, tim2);
    tim2.enable_update_interrupt();
    EXPECT_EQ(TIM1_BASE->DIER & TIM_DIER_UIE, TIM_DIER_UIE);
    tim2.disable_update_interrupt();
    EXPECT_EQ(TIM1_BASE->DIER & TIM_DIER_UIE, 0);
}

TEST_F(TimerWrapperTests, NVIC_EnabledDisabled) {
    auto testBoard = ST_LIB::Board<tim2>;
    testBoard.init();

    auto tim2 = get_timer_instance(testBoard, tim2);
    tim2.enable_nvic();
    EXPECT_EQ(NVIC_GetEnableIRQ(TIM2_IRQn), 1);
    tim2.disable_nvic();
    EXPECT_EQ(NVIC_GetEnableIRQ(TIM2_IRQn), 0);
}

TEST_F(TimerWrapperTests, UpdateEvent_EnabledDisabled) {
    auto testBoard = ST_LIB::Board<tim2>;
    testBoard.init();

    auto tim2 = get_timer_instance(testBoard, tim2);
    tim2.enable_update_event();
    EXPECT_EQ(TIM1_BASE->CR1 & TIM_CR1_UDIS, 0);
    tim2.disable_update_event();
    EXPECT_EQ(TIM1_BASE->CR1 & TIM_CR1_UDIS, TIM_CR1_UDIS);
}

TEST_F(TimerWrapperTests, BreakInterrupt_EnabledDisabled) {
    auto testBoard = ST_LIB::Board<tim2>;
    testBoard.init();

    auto tim2 = get_timer_instance(testBoard, tim2);
    tim2.break_interrupt_enable();
    EXPECT_EQ(TIM1_BASE->DIER & TIM_DIER_BIE, TIM_DIER_BIE);
    tim2.break_interrupt_disable();
    EXPECT_EQ(TIM1_BASE->DIER & TIM_DIER_BIE, 0);
}

TEST_F(TimerWrapperTests, BreakInterrupt_EnabledDisabled) {
    auto testBoard = ST_LIB::Board<tim2>;
    testBoard.init();

    auto tim2 = get_timer_instance(testBoard, tim2);
    tim2.set_one_pulse_mode();
    EXPECT_EQ(TIM1_BASE->CR1 & TIM_CR1_OPM, TIM_CR1_OPM);
    tim2.set_multi_interrupt_mode();
    EXPECT_EQ(TIM1_BASE->CR1 & TIM_CR1_OPM, 0);
}

void callback(void *raw) {}

TEST_F(TimerWrapperTests, ConfigureTimer) {
    auto testBoard = ST_LIB::Board<tim2>;
    testBoard.init();

#define PRESCALER_VAL 200
#define PERIOD 1000
    auto tim2 = get_timer_instance(testBoard, tim2);
    tim2.configure16bit<PRESCALER_VAL>(callback, 0, PERIOD);
    EXPECT_EQ(TIM1_BASE->PSC, PRESCALER_VAL); /* set prescaler */
    EXPECT_EQ(TIM1_BASE->ARR, PERIOD); /* set period */
    EXPECT_EQ(TIM1_BASE->CR1 & TIM_CR1_CEN, TIM_CR1_CEN); /* set counter enable */
}

