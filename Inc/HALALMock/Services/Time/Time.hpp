/*
 * Time.hpp
 *
 *  Created on: 23 oct. 2024
 *      Author: Stefan Costea
 */

#pragma once

#include <chrono>
#include <functional>
#include <map>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <queue>
#include <set>
#include <cstdint>
#include <stdexcept>

class Time {
private:
    using RealClock = std::chrono::steady_clock;
    using RealTimePoint = RealClock::time_point;
    using SimDuration = std::chrono::nanoseconds;

    struct Alarm {
        SimDuration period;
        std::function<void()> callback;
        SimDuration next_trigger;
        bool active;
        bool erase;
    };

    static std::mutex mutex;
    static std::condition_variable cv;
    static std::thread worker_thread;
    static std::atomic<bool> running;
    static std::map<uint8_t, Alarm> alarms;
    static uint8_t next_alarm_id;

    static RealTimePoint simulation_start_time;
    static SimDuration simulation_time;
	static SimDuration lag;			//Difference of time between expected time and simulated time
    static double simulation_speed; // The ratio between simulation time and real time

    static void worker();

public:
    static void start(double simulation_speed = 1.0);
    static void stop();

    static uint64_t get_global_tick();

    static uint8_t register_high_precision_alarm(uint32_t period_in_us, std::function<void()> func);
    static bool unregister_high_precision_alarm(uint8_t id);

    static uint8_t register_low_precision_alarm(uint32_t period_in_ms, std::function<void()> func);
    static bool unregister_low_precision_alarm(uint8_t id);

    static uint8_t set_timeout(uint32_t milliseconds, std::function<void()> callback);
    static void cancel_timeout(uint8_t id);

    // Function to return simulation lag
    static SimDuration get_lag();
};
