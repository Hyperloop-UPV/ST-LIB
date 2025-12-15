/*
 * Scheduler.hpp
 *
 * Created on: 17 nov. 2025
 *     Author: Victor (coauthor Stephan)
 */
#pragma once

#ifndef TESTING_ENV
    #include "stm32h7xx_ll_tim.h"
#else
    #include "MockedDrivers/ll_tim_interface.h"
#endif
#include <array>
#include <cstdint>
#include <functional>

/* NOTE(vic): Pido perdón a Boris pero es la mejor manera que se me ha ocurrido hacer esto
 *  Cambiar el SCHEDULER_TIMER_IDX si es mejor usar otro timer que no sea TIM2
 */
#ifndef SCHEDULER_TIMER_IDX
# define SCHEDULER_TIMER_IDX 2
#endif

#define glue_(a,b) a ## b
#define glue(a,b) glue_(a,b)
#define SCHEDULER_TIMER_BASE glue(TIM, glue(SCHEDULER_TIMER_IDX, _BASE))

    // Used to reserve a TimerPeripheral
#ifndef TESTING_ENV
#include "stm32h7xx_hal_tim.h"
#define SCHEDULER_HAL_TIM glue(htim, SCHEDULER_TIMER_IDX)
extern TIM_HandleTypeDef SCHEDULER_HAL_TIM;
#endif

struct Scheduler {
    using callback_t = void (*)();
    static constexpr uint32_t INVALID_ID = 0xFFu;

    static void start();
    static void update();
    static inline uint64_t get_global_tick();

    static inline uint8_t register_task(uint32_t period_us, callback_t func) {
        if(period_us == 0) [[unlikely]] period_us = 1;
        return register_task(period_us, func, true);
    }
    static bool unregister_task(uint8_t id);

    static inline uint8_t set_timeout(uint32_t microseconds, callback_t func) {
        if(microseconds == 0) [[unlikely]] microseconds = 1;
        return register_task(microseconds, func, false);
    }
    static inline bool cancel_timeout(uint8_t id) {
        /* NOTE: This does not fix this case:
          1. id = set_timeout(x, func)
          2. timeout ends, func gets called and removed internally
          3. id_2 = set_timeout(y, func_2) // id will be equal to id_2
          4. clear_timeout(id) -> will remove the second timeout
         */
        if(tasks_[id].repeating) return false;
        return unregister_task(id);
    }

    // static void global_timer_callback();

    // Have to be public because SCHEDULER_GLOBAL_TIMER_CALLBACK won't work otherwise
    //static const uint32_t global_timer_base = SCHEDULER_TIMER_BASE;
    static void on_timer_update();

#ifndef TESTING_ENV
    private:
#endif
    struct Task {
        uint64_t next_fire_us{0};
        callback_t callback{};
        uint32_t period_us{0};
        bool repeating{false};
    };

    static constexpr std::size_t kMaxTasks = 16;
    static_assert((kMaxTasks & (kMaxTasks - 1)) == 0, "kMaxTasks must be a power of two");
    static constexpr uint32_t FREQUENCY = 1'000'000u; // 1 MHz -> 1us precision

    static std::array<Task, kMaxTasks> tasks_;
    static_assert(kMaxTasks == 16, "kMaxTasks must be 16, if more is needed, sorted_task_ids_ must change");
    /* sorted_task_ids_ is a sorted queue with 4bits for each id in the scheduler's current ids */
    static uint64_t sorted_task_ids_;

    static uint32_t active_task_count_;
    static_assert(kMaxTasks <= 32, "kMaxTasks must be <= 32, if more is needed, the bitmaps must change");
    static uint32_t ready_bitmap_;
    static uint32_t free_bitmap_;
    static uint64_t global_tick_us_;
    static uint32_t current_interval_us_;

    static inline uint8_t allocate_slot();
    static inline void release_slot(uint8_t id);
    static void insert_sorted(uint8_t id);
    static void remove_sorted(uint8_t id);
    static void schedule_next_interval();
    static inline void configure_timer_for_interval(uint64_t microseconds);
    static uint8_t register_task(uint32_t period_us, callback_t func, bool repeating);

    // helpers
    static inline uint8_t get_at(uint8_t idx);
    static inline void set_at(uint8_t idx, uint8_t id);
    static inline void pop_front();
    static inline uint8_t front_id();

    static inline void global_timer_disable();
    static inline void global_timer_enable();
};
