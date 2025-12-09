#include <gtest/gtest.h>
#include <thread>
#include<chrono>
#include "HALAL/Services/Time/Scheduler.hpp"

int count = 0;
void fake_workload(){
    count++;
}

class SchedulerTests : public ::testing::Test {
protected:
    void SetUp() override {
        Scheduler::active_task_count_ = 0;
        Scheduler::free_bitmap_ = 0xFFFF'FFFF;
        Scheduler::ready_bitmap_ = 0;
        Scheduler::sorted_task_ids_ = 0;
        Scheduler::global_tick_us_ = 0;
        Scheduler::current_interval_us_ = 0;
        
        // Reset global callback task count
        count = 0;
        
        // Reset Timer
        TIM2_BASE->CNT = 0;
        TIM2_BASE->ARR = 0;
        TIM2_BASE->SR = 0;
        TIM2_BASE->CR1 = 0;
        TIM2_BASE->DIER = 0;
    }
};

TEST_F(SchedulerTests, FreeBitmap) {
    Scheduler::register_task(10,&fake_workload);
    EXPECT_EQ(Scheduler::free_bitmap_, 0xFFFF'FFFE);
}

TEST_F(SchedulerTests, TaskRegistration) {
    Scheduler::register_task(10,&fake_workload);
    EXPECT_EQ(Scheduler::tasks_[0].callback,fake_workload);
}

TEST_F(SchedulerTests, TaskExecutionShort) {
    Scheduler::register_task(10,&fake_workload);
    Scheduler::start();
    // TIM2_BASE->ARR = 500; 
    // TIM2_BASE->generate_update();
    
    constexpr int NUM_TICKS = 1'000;
    for(int i = 0; i < NUM_TICKS; i++){
        TIM2_BASE->CNT++;
        Scheduler::update();
    }
    // 1000 ticks / 10 ticks/task = 100 executions.
    EXPECT_EQ(count, 100);
}

TEST_F(SchedulerTests, TaskExecutionLong) {
    Scheduler::register_task(10,&fake_workload);
    Scheduler::start();
    // TIM2_BASE->ARR = 500;
    // TIM2_BASE->generate_update();
    
    constexpr int NUM_TICKS = 1'000'000;
    for(int i = 0; i < NUM_TICKS; i++){
        TIM2_BASE->CNT++;
        Scheduler::update();
    }
    EXPECT_EQ(count, 100'000);
}

TEST_F(SchedulerTests, SetTimeout) {
    Scheduler::set_timeout(10, &fake_workload);
    Scheduler::start();
    
    constexpr int NUM_TICKS = 100;
    for(int i = 0; i < NUM_TICKS; i++){
        TIM2_BASE->CNT++;
        Scheduler::update();
    }
    EXPECT_EQ(count, 1);
}

TEST_F(SchedulerTests, GlobalTickOverflow) {
    Scheduler::global_tick_us_ = 0xFFFFFFF0ULL; // Near 32-bit max
    Scheduler::register_task(20, &fake_workload);
    Scheduler::start();
    
    constexpr int NUM_TICKS = 100;
    for(int i = 0; i < NUM_TICKS; i++){
        TIM2_BASE->CNT++;
        Scheduler::update();
    }
    // 100 ticks /20 ticks/task = 5 executions.
    EXPECT_EQ(count, 5);
}
