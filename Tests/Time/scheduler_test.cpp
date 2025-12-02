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
    for(int i = 0; i < NUM_TICKS; i++){
        Scheduler::simulate_ticking();
        Scheduler::update();
    }
    EXPECT_EQ(count,100'000);
}