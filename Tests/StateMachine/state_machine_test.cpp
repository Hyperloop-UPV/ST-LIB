#include <gtest/gtest.h>
#include "ST-LIB_LOW/StateMachine/StateMachine.hpp"
#include "HALAL/Services/Time/Scheduler.hpp"
#include "HALAL/Models/TimerDomain/TimerDomain.hpp"

enum class MasterState { A, B, C };

enum class SubState { S1, S2 };

static int a_enter_count = 0;
static int a_exit_count = 0;
static int a_cyclic_count = 0;

static int b_enter_count = 0;
static int b_exit_count = 0;
static int b_cyclic_count = 0;

static int c_enter_count = 0;
static int c_exit_count = 0;

static int s1_enter_count = 0;
static int s1_exit_count = 0;
static int s1_cyclic_count = 0;

static int s2_enter_count = 0;
static int s2_exit_count = 0;

static bool condition_a_to_b = false;
static bool condition_b_to_c = false;
static bool condition_c_to_a = false;
static bool condition_s1_to_s2 = false;

static void reset_test_state() {
    a_enter_count = 0;
    a_exit_count = 0;
    a_cyclic_count = 0;

    b_enter_count = 0;
    b_exit_count = 0;
    b_cyclic_count = 0;

    c_enter_count = 0;
    c_exit_count = 0;

    s1_enter_count = 0;
    s1_exit_count = 0;
    s1_cyclic_count = 0;

    s2_enter_count = 0;
    s2_exit_count = 0;

    condition_a_to_b = false;
    condition_b_to_c = false;
    condition_c_to_a = false;
    condition_s1_to_s2 = false;

    Scheduler::active_task_count_ = 0;
    Scheduler::free_bitmap_ = 0xFFFF'FFFF;
    Scheduler::ready_bitmap_ = 0;
    Scheduler::sorted_task_ids_ = 0;
    Scheduler::global_tick_us_ = 0;
    Scheduler::current_interval_us_ = 0;

    TIM2_BASE->CNT = 0;
    TIM2_BASE->ARR = 0;
    TIM2_BASE->SR = 0;
    TIM2_BASE->CR1 = 0;
    TIM2_BASE->DIER = 0;

    Scheduler_global_timer =
        ST_LIB::TimerDomain::cmsis_timers[ST_LIB::timer_idxmap[SCHEDULER_TIMER_DOMAIN]];
}

static void tick_scheduler(int ticks) {
    TIM2_BASE->PSC = 2;
    for (int i = 0; i < ticks; i++) {
        for (int j = 0; j <= TIM2_BASE->PSC; j++)
            TIM2_BASE->inc_cnt_and_check(1);
        Scheduler::update();
    }
}

static constexpr auto state_s1 =
    make_state(SubState::S1, Transition<SubState>{SubState::S2, []() {
                                                      return condition_s1_to_s2;
                                                  }});
static constexpr auto state_s2 = make_state(SubState::S2);

static inline auto test_nested_machine = []() consteval {
    auto sm = make_state_machine(SubState::S1, state_s1, state_s2);
    using namespace std::chrono_literals;

    sm.add_enter_action([]() { s1_enter_count++; }, state_s1);
    sm.add_exit_action([]() { s1_exit_count++; }, state_s1);
    sm.add_cyclic_action([]() { s1_cyclic_count++; }, 10ms, state_s1);

    sm.add_enter_action([]() { s2_enter_count++; }, state_s2);
    sm.add_exit_action([]() { s2_exit_count++; }, state_s2);

    return sm;
}();

static constexpr auto state_a =
    make_state(MasterState::A, Transition<MasterState>{MasterState::B, []() {
                                                           return condition_a_to_b;
                                                       }});
static constexpr auto state_b =
    make_state(MasterState::B, Transition<MasterState>{MasterState::C, []() {
                                                           return condition_b_to_c;
                                                       }});
static constexpr auto state_c =
    make_state(MasterState::C, Transition<MasterState>{MasterState::A, []() {
                                                           return condition_c_to_a;
                                                       }});

static inline auto test_machine = []() consteval {
    auto nested = StateMachineHelper::add_nesting(state_b, test_nested_machine);
    auto sm = make_state_machine(
        MasterState::A,
        StateMachineHelper::add_nested_machines(nested),
        state_a,
        state_b,
        state_c
    );
    using namespace std::chrono_literals;

    sm.add_enter_action([]() { a_enter_count++; }, state_a);
    sm.add_exit_action([]() { a_exit_count++; }, state_a);
    sm.add_cyclic_action([]() { a_cyclic_count++; }, 10ms, state_a);

    sm.add_enter_action([]() { b_enter_count++; }, state_b);
    sm.add_exit_action([]() { b_exit_count++; }, state_b);
    sm.add_cyclic_action([]() { b_cyclic_count++; }, 20ms, state_b);

    sm.add_enter_action([]() { c_enter_count++; }, state_c);
    sm.add_exit_action([]() { c_exit_count++; }, state_c);

    return sm;
}();

class StateMachineTest : public ::testing::Test {
protected:
    void SetUp() override {
        ::ST_LIB::ClockDomain::s_tree = &::ST_LIB::default_clock_tree;
        reset_test_state();

        test_machine.force_change_state((size_t)MasterState::A);
        test_nested_machine.force_change_state((size_t)SubState::S1);

        test_machine.force_change_state((size_t)MasterState::A);
        test_nested_machine.force_change_state((size_t)SubState::S1);
        test_machine.get_states()[0].unregister_all_timed_actions();
        test_machine.get_states()[1].unregister_all_timed_actions();
        test_machine.get_states()[2].unregister_all_timed_actions();
        test_nested_machine.get_states()[0].unregister_all_timed_actions();
        test_nested_machine.get_states()[1].unregister_all_timed_actions();

        reset_test_state();

        Scheduler_start();
    }
};

TEST_F(StateMachineTest, StartTriggersEnterActions) {
    test_machine.start();
    EXPECT_EQ(a_enter_count, 1);
    EXPECT_EQ(a_exit_count, 0);
    EXPECT_EQ(b_enter_count, 0);
    EXPECT_EQ(test_machine.get_current_state(), MasterState::A);
}

TEST_F(StateMachineTest, BasicTransition) {
    test_machine.start();
    a_enter_count = 0;

    condition_a_to_b = true;
    test_machine.check_transitions();

    EXPECT_EQ(test_machine.get_current_state(), MasterState::B);
    EXPECT_EQ(a_exit_count, 1);
    EXPECT_EQ(b_enter_count, 1);
    EXPECT_EQ(s1_enter_count, 1); // Nested machine should also enter its initial state
}

TEST_F(StateMachineTest, NestedTransition) {
    test_machine.start();
    condition_a_to_b = true;
    test_machine.check_transitions();

    EXPECT_EQ(test_machine.get_current_state(), MasterState::B);
    EXPECT_EQ(test_nested_machine.get_current_state(), SubState::S1);

    condition_s1_to_s2 = true;
    test_machine.check_transitions();

    EXPECT_EQ(test_nested_machine.get_current_state(), SubState::S2);
    EXPECT_EQ(s1_exit_count, 1);
    EXPECT_EQ(s2_enter_count, 1);
}

TEST_F(StateMachineTest, MasterStateChangeExitsNested) {
    test_machine.start();
    condition_a_to_b = true;
    test_machine.check_transitions();

    EXPECT_EQ(test_nested_machine.get_current_state(), SubState::S1);
    s1_enter_count = 0;
    b_exit_count = 0;

    condition_b_to_c = true;
    test_machine.check_transitions();

    EXPECT_EQ(test_machine.get_current_state(), MasterState::C);
    EXPECT_EQ(b_exit_count, 1);
    EXPECT_EQ(s1_exit_count, 1);
    EXPECT_EQ(c_enter_count, 1);
}

TEST_F(StateMachineTest, CyclicActionsRun) {
    test_machine.start();

    tick_scheduler(100);
    tick_scheduler(10000);

    EXPECT_GE(a_cyclic_count, 1);
    EXPECT_EQ(b_cyclic_count, 0);

    condition_a_to_b = true;
    test_machine.check_transitions();

    a_cyclic_count = 0;

    tick_scheduler(20000);

    EXPECT_EQ(a_cyclic_count, 0);  // A cyclic shouldn't run
    EXPECT_GE(b_cyclic_count, 1);  // B cyclic should run
    EXPECT_GE(s1_cyclic_count, 1); // Nested S1 cyclic should run
}

static int custom_task_count = 0;

static int check_transition_task_count = 0;

TEST_F(StateMachineTest, StressTestWithScheduler) {
    test_machine.start();

    custom_task_count = 0;
    check_transition_task_count = 0;

    auto custom_task_id = Scheduler::register_task(999, []() { custom_task_count++; });

    auto check_task_id = Scheduler::register_task(499, []() {
        test_machine.check_transitions();
        check_transition_task_count++;
    });

    for (int i = 0; i < 50; i++) {
        // A -> B
        condition_a_to_b = true;
        condition_b_to_c = false;
        condition_c_to_a = false;
        tick_scheduler(1500);

        // B -> C
        condition_a_to_b = false;
        condition_b_to_c = true;
        condition_c_to_a = false;
        tick_scheduler(1500);

        // C -> A
        condition_a_to_b = false;
        condition_b_to_c = false;
        condition_c_to_a = true;
        tick_scheduler(1500);
    }

    condition_a_to_b = false;
    condition_b_to_c = false;
    condition_c_to_a = false;

    EXPECT_GT(custom_task_count, 100);
    EXPECT_GT(check_transition_task_count, 300);

    // Because state changes were strictly faster than 10ms, no cyclic tasks should have fired
    EXPECT_EQ(a_cyclic_count, 0);
    EXPECT_EQ(b_cyclic_count, 0);

    tick_scheduler(20000); // 20ms
    EXPECT_GE(a_cyclic_count, 1);

    condition_a_to_b = true;
    tick_scheduler(40000); // 40ms in State B
    EXPECT_GE(b_cyclic_count, 1);

    EXPECT_LE(Scheduler::active_task_count_, 5);

    Scheduler::unregister_task(custom_task_id);
    Scheduler::unregister_task(check_task_id);
}

enum class StartAtOne { A = 1, B = 2 };
enum class NonContiguous { A = 0, B = 2 };

template <auto V> struct constant_eval {};

template <typename F>
concept CanCompile = requires { typename constant_eval<F::invoke()>; };

struct DuplicateNestedCheck {
    static consteval bool invoke() {
        auto sm_nested_1 = make_state_machine(SubState::S1, state_s1, state_s2);
        auto sm_nested_2 = make_state_machine(SubState::S1, state_s1, state_s2);

        auto nested1 = StateMachineHelper::add_nesting(state_a, sm_nested_1);
        auto nested2 = StateMachineHelper::add_nesting(state_a, sm_nested_2);

        auto sm = make_state_machine(
            MasterState::A,
            StateMachineHelper::add_nested_machines(nested1, nested2),
            state_a,
            state_b
        );
        return true;
    }
};

struct ValidNestedCheck {
    static consteval bool invoke() {
        auto sm_nested_1 = make_state_machine(SubState::S1, state_s1, state_s2);
        auto sm_nested_2 = make_state_machine(SubState::S1, state_s1, state_s2);

        auto nested1 = StateMachineHelper::add_nesting(state_a, sm_nested_1);
        auto nested2 = StateMachineHelper::add_nesting(state_b, sm_nested_2);

        auto sm = make_state_machine(
            MasterState::A,
            StateMachineHelper::add_nested_machines(nested1, nested2),
            state_a,
            state_b
        );
        return true;
    }
};

struct StartAtOneCheck {
    static consteval bool invoke() {
        auto stA = make_state(StartAtOne::A);
        auto stB = make_state(StartAtOne::B);
        auto sm = make_state_machine(StartAtOne::A, stA, stB);
        return true;
    }
};

struct NonContiguousCheck {
    static consteval bool invoke() {
        auto stA = make_state(NonContiguous::A);
        auto stB = make_state(NonContiguous::B);
        auto sm = make_state_machine(NonContiguous::A, stA, stB);
        return true;
    }
};

struct DuplicateStateInConstructorCheck {
    static consteval bool invoke() {
        auto stA = make_state(MasterState::A);
        auto sm = make_state_machine(MasterState::A, stA, stA);
        return true;
    }
};

struct SelfTransitionCheck {
    static consteval bool invoke() {
        auto stA = make_state(MasterState::A, Transition<MasterState>{MasterState::A, [] {
                                                                          return true;
                                                                      }});
        return true;
    }
};

struct InvalidActionStateCheck {
    static consteval bool invoke() {
        auto stA = make_state(MasterState::A);
        auto sm = make_state_machine(MasterState::A, stA);
        auto stB = make_state(MasterState::B);
        sm.add_enter_action([] {}, stB);
        return true;
    }
};

TEST(StateMachineCompileCheck, ValidatesSFINAEOntoS_M) {
    static_assert(CanCompile<ValidNestedCheck>, "Valid nested mapping should compile.");
    static_assert(!CanCompile<DuplicateNestedCheck>, "Duplicate state mappings must not compile.");
    static_assert(!CanCompile<StartAtOneCheck>, "State Enums must start at 0.");
    static_assert(!CanCompile<NonContiguousCheck>, "State Enums must be contiguous.");
    static_assert(
        !CanCompile<DuplicateStateInConstructorCheck>,
        "Must not allow duplicate states in Constructor."
    );
    static_assert(!CanCompile<SelfTransitionCheck>, "Self transition must not compile.");
    static_assert(
        !CanCompile<InvalidActionStateCheck>,
        "Adding action to invalid state must not compile."
    );
}
