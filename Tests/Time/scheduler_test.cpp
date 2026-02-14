#include <gtest/gtest.h>
#include <thread>
#include <chrono>

#include "HALAL/Services/Time/Scheduler.hpp"

int count = 0;
void fake_workload() { count++; }

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
    Scheduler::register_task(10, &fake_workload);
    EXPECT_EQ(Scheduler::free_bitmap_, 0xFFFF'FFFE);
}

TEST_F(SchedulerTests, TaskRegistration) {
    Scheduler::register_task(10, &fake_workload);
    EXPECT_EQ(Scheduler::tasks_[0].callback, fake_workload);
}

TEST_F(SchedulerTests, TaskExecutionShort) {
    Scheduler::register_task(10, &fake_workload);
    Scheduler::start();
    TIM2_BASE->PSC = 2; // quicker test

    constexpr int NUM_TICKS = 1'000;
    for (int i = 0; i < NUM_TICKS; i++) {
        for (int j = 0; j <= TIM2_BASE->PSC; j++)
            TIM2_BASE->inc_cnt_and_check(1);
        Scheduler::update();
    }
    // 1000 ticks / 10 ticks/task = 100 executions.
    EXPECT_EQ(count, 100);
}

TEST_F(SchedulerTests, TaskExecutionLong) {
    Scheduler::register_task(10, &fake_workload);
    Scheduler::start();
    // TIM2_BASE->ARR = 500;
    TIM2_BASE->generate_update();
    TIM2_BASE->PSC = 2; // quicker test

    constexpr int NUM_TICKS = 1'000'000;
    for (int i = 0; i < NUM_TICKS; i++) {
        for (int j = 0; j <= TIM2_BASE->PSC; j++)
            TIM2_BASE->inc_cnt_and_check(1);
        Scheduler::update();
    }
    EXPECT_EQ(count, 100'000);
}

TEST_F(SchedulerTests, SetTimeout) {
    Scheduler::set_timeout(10, &fake_workload);
    Scheduler::start();
    TIM2_BASE->PSC = 2; // quicker test

    constexpr int NUM_TICKS = 100;
    for (int i = 0; i < NUM_TICKS; i++) {
        for (int j = 0; j <= TIM2_BASE->PSC; j++)
            TIM2_BASE->inc_cnt_and_check(1);
        Scheduler::update();
    }
    EXPECT_EQ(count, 1);
}

TEST_F(SchedulerTests, GlobalTickOverflow) {
    Scheduler::global_tick_us_ = 0xFFFFFFF0ULL; // Near 32-bit max
    Scheduler::register_task(20, &fake_workload);
    Scheduler::start();
    TIM2_BASE->PSC = 2; // quicker test

    constexpr int NUM_TICKS = 100;
    for (int i = 0; i < NUM_TICKS; i++) {
        for (int j = 0; j <= TIM2_BASE->PSC; j++)
            TIM2_BASE->inc_cnt_and_check(1);

        Scheduler::update();
    }
    // 100 ticks /20 ticks/task = 5 executions.
    EXPECT_EQ(count, 5);
}

#define multiple_tasks                                                                             \
    X(1)                                                                                           \
    X(2)                                                                                           \
    X(3)                                                                                           \
    X(4)                                                                                           \
    X(5)                                                                                           \
    X(6)                                                                                           \
    X(7)                                                                                           \
    X(8)                                                                                           \
    X(9)                                                                                           \
    X(10)                                                                                          \
    X(11)                                                                                          \
    X(12)                                                                                          \
    X(13)                                                                                          \
    X(14)                                                                                          \
    X(15)                                                                                          \
    X(16)

#define X(n)                                                                                       \
    int multiple_task##n##count = 0;                                                               \
    void multiple_task_##n(void) { multiple_task##n##count++; }
multiple_tasks
#undef X

TEST_F(SchedulerTests, GlobalTickOverflowManyTasks) {
    Scheduler::global_tick_us_ = 0xFFFFFFF0ULL; // Near 32-bit max
    Scheduler::register_task(10, &multiple_task_1);
    Scheduler::register_task(20, &multiple_task_2);
    Scheduler::register_task(30, &multiple_task_3);
    Scheduler::start();
    TIM2_BASE->PSC = 2; // quicker test

    constexpr int NUM_TICKS = 100;
    for (int i = 0; i < NUM_TICKS; i++) {
        for (int j = 0; j <= TIM2_BASE->PSC; j++)
            TIM2_BASE->inc_cnt_and_check(1);

        Scheduler::update();
    }
    // 100 ticks /20 ticks/task = 5 executions.
    EXPECT_EQ(multiple_task1count, NUM_TICKS / 10);
    EXPECT_EQ(multiple_task2count, NUM_TICKS / 20);
    EXPECT_EQ(multiple_task3count, NUM_TICKS / 30);
}

TEST_F(SchedulerTests, TimeoutClearAddTask) {
    uint8_t timeout_id = Scheduler::set_timeout(10, &fake_workload);
    Scheduler::start();
    TIM2_BASE->PSC = 2; // quicker test

    constexpr int NUM_TICKS = 100;
    for (int i = 0; i < NUM_TICKS; i++) {
        for (int j = 0; j <= TIM2_BASE->PSC; j++)
            TIM2_BASE->inc_cnt_and_check(1);
        Scheduler::update();
    }

    // timeout is already done here
    uint8_t timeout_id_2 = Scheduler::set_timeout(20, &fake_workload);

    // after timeout, cancel task
    Scheduler::cancel_timeout(timeout_id);

    EXPECT_EQ(Scheduler::active_task_count_, 1);
}

int tickidx = 0;
static volatile int connecting_execs{0};
static volatile int operational_execs{0};
static volatile int fault_execs{0};
void connecting_cyclic() {
    auto next_connecting_execs = connecting_execs + 1;
    connecting_execs = next_connecting_execs;
}
void operational_cyclic() {
    auto next_operational_execs = operational_execs + 1;
    operational_execs = next_operational_execs;
}
void fault_cyclic() {
    auto next_fault_execs = fault_execs + 1;
    fault_execs = next_fault_execs;
}
TEST_F(SchedulerTests, TaskDe_ReRegistration) {
    uint8_t connecting_task = Scheduler::register_task(10, &connecting_cyclic);
    uint8_t operational_task = 0;
    uint8_t fault_task = 0;
    Scheduler::start();
    TIM2_BASE->PSC = 2; // quicker test

    constexpr int NUM_TICKS = 100;
    for (; tickidx < NUM_TICKS; tickidx++) {
        for (int j = 0; j <= TIM2_BASE->PSC; j++)
            TIM2_BASE->inc_cnt_and_check(1);
        if (tickidx == 21) {
            operational_task = Scheduler::register_task(10, operational_cyclic);
            Scheduler::unregister_task(connecting_task);
        }
        if (tickidx == 45) {
            fault_task = Scheduler::register_task(10, fault_cyclic);
            Scheduler::unregister_task(operational_task);
        }
        if (tickidx == 70) {
            Scheduler::unregister_task(fault_task);
            tickidx = 100; // finish test
        }
        Scheduler::update();
    }
    EXPECT_EQ(connecting_execs, 2);
    EXPECT_EQ(operational_execs, 2);
    EXPECT_EQ(fault_execs, 2);
}

TEST_F(SchedulerTests, MultipleTasks) {
    multiple_task1count = 0;
    multiple_task2count = 0;
    multiple_task3count = 0;
    multiple_task4count = 0;
    multiple_task5count = 0;
    multiple_task6count = 0;

    Scheduler::register_task(1, &multiple_task_1);
    Scheduler::register_task(2, &multiple_task_2);
    Scheduler::register_task(3, &multiple_task_3);
    Scheduler::register_task(4, &multiple_task_4);
    Scheduler::register_task(5, &multiple_task_5);
    Scheduler::register_task(6, &multiple_task_6);

    Scheduler::start();
    TIM2_BASE->PSC = 2; // quicker test
    constexpr int NUM_TICKS = 300;
    for (int i = 0; i < NUM_TICKS; i++) {
        Scheduler::update();
        for (int j = 0; j <= TIM2_BASE->PSC; j++)
            TIM2_BASE->inc_cnt_and_check(1);
    }

    EXPECT_EQ(multiple_task1count, NUM_TICKS / 1 - 1);
    EXPECT_EQ(multiple_task2count, NUM_TICKS / 2 - 1);
    EXPECT_EQ(multiple_task3count, NUM_TICKS / 3 - 1);
    EXPECT_EQ(multiple_task4count, NUM_TICKS / 4 - 1);
    EXPECT_EQ(multiple_task5count, NUM_TICKS / 5 - 1);
    EXPECT_EQ(multiple_task6count, NUM_TICKS / 6 - 1);
}

TEST_F(SchedulerTests, SameTaskMultipleTimes) {
    multiple_task1count = 0;
    Scheduler::register_task(1, &multiple_task_1);
    Scheduler::register_task(2, &multiple_task_1);
    Scheduler::register_task(3, &multiple_task_1);
    Scheduler::register_task(4, &multiple_task_1);
    Scheduler::register_task(5, &multiple_task_1);
    Scheduler::register_task(6, &multiple_task_1);

    multiple_task1count = 0;
    Scheduler::start();
    TIM2_BASE->PSC = 2; // quicker test
    constexpr int NUM_TICKS = 300;
    for (int i = 0; i < NUM_TICKS; i++) {
        Scheduler::update();
        for (int j = 0; j <= TIM2_BASE->PSC; j++)
            TIM2_BASE->inc_cnt_and_check(1);
    }

    EXPECT_EQ(
        multiple_task1count,
        NUM_TICKS / 1 - 1 + NUM_TICKS / 2 - 1 + NUM_TICKS / 3 - 1 + NUM_TICKS / 4 - 1 +
            NUM_TICKS / 5 - 1 + NUM_TICKS / 6 - 1
    );
}
