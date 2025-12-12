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
    TIM2_BASE->generate_update();
    
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

TEST_F(SchedulerTests, TimeoutClearAddTask) {
    uint8_t timeout_id = Scheduler::set_timeout(10, &fake_workload);
    Scheduler::start();

    constexpr int NUM_TICKS = 100;
    for(int i = 0; i < NUM_TICKS; i++) {
        TIM2_BASE->CNT++;
        Scheduler::update();
    }

    // timeout is already done here
    uint8_t task_id = Scheduler::register_task(20, &fake_workload);
    
    // after timeout, cancel task
    Scheduler::cancel_timeout(timeout_id);

    EXPECT_EQ(Scheduler::active_task_count_, 1);
}

static volatile int connecting_execs{0};
static volatile int operational_execs{0};
static volatile int fault_execs{0}; 
void connecting_cyclic(){
    connecting_execs++;
}
void operational_cyclic(){
    operational_execs++;
}
void fault_cyclic(){
    fault_execs++;
}
TEST_F(SchedulerTests, TaskDe_ReRegistration) {
    uint8_t connecting_task = Scheduler::register_task(10, &connecting_cyclic);
    uint8_t operational_task = 0;
    uint8_t fault_task = 0;
    Scheduler::start();

    constexpr int NUM_TICKS = 100;
    for(int i = 0; i < NUM_TICKS; i++) {
        TIM2_BASE->CNT++;
        if(i == 21){
            Scheduler::unregister_task(connecting_task);
            operational_task = Scheduler::register_task(10,operational_cyclic);
        }
        if(i == 45){
            Scheduler::unregister_task(operational_task);
            fault_task = Scheduler::register_task(10,fault_cyclic);
        }
        if( i == 70){
            Scheduler::unregister_task(fault_task);
             i = 100; // finish test
        }
        Scheduler::update();
    }
    EXPECT_EQ(connecting_execs, 2);
    EXPECT_EQ(operational_execs, 2);
    EXPECT_EQ(fault_execs, 2);
}
