#include <gtest/gtest.h>
#include <thread>
#include<chrono>
#include "HALAL/Services/Time/Scheduler.hpp"
int count = 0;
void fake_workload(){
    count++;
}

TEST(SchedulerTests, UsedBitmap) {
    Scheduler::register_task(10,&fake_workload);
    EXPECT_EQ(Scheduler::used_bitmap_,1);
}

TEST(SchedulerTests, TaskRegistration) {
    Scheduler::register_task(10,&fake_workload);
    EXPECT_EQ(Scheduler::tasks_[0].callback,fake_workload);
}

TEST(SchedulerTests, TaskExecution) {
    Scheduler::register_task(10,&fake_workload);
    Scheduler::start();
    constexpr int NUM_TICKS = 1'000'000;
    TIM2_BASE->ARR = 500;
    TIM2_BASE->generate_update();
    for(int i = 0; i <= NUM_TICKS; i++){
        TIM2_BASE->CNT++;
        Scheduler::update();
    }
    // one tick is 1us, and we register a task that executes every 10us
    // thus it should execute NUM_TICKS/10
    EXPECT_EQ(count,100'000);
}