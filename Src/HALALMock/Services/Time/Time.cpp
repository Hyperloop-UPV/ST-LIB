/*
 * Time.cpp
 *
 *  Created on: 23 oct. 2024
 *      Author: Stefan Costea
 */


#include <iostream>
#include "HALALMock/Services/Time/Time.hpp"

std::mutex Time::mutex;
std::condition_variable Time::cv;
std::thread Time::worker_thread;
std::atomic<bool> Time::running{false};
std::map<uint8_t, Time::Alarm> Time::alarms;
uint8_t Time::next_alarm_id = 0;


Time::RealTimePoint Time::simulation_start_time;
Time::SimDuration Time::simulation_time{0};
Time::SimDuration Time::lag{0};
double Time::simulation_speed = 1.0;

std::chrono::microseconds max_lag = std::chrono::microseconds(100);

void Time::start(double simulation_speed) {
    std::lock_guard<std::mutex> lock(mutex);
    if (running) {
        return; // Already running
    }
    if (simulation_speed <= 0.0) {
        //TODO: ErrorHandler("Simulation speed must be positive");
    }
    Time::simulation_speed = simulation_speed;
    simulation_start_time = RealClock::now();
    simulation_time = SimDuration(0);
    lag = SimDuration(0);
    running = true;
    worker_thread = std::thread(worker);
}

void Time::stop() {
    {
        std::lock_guard<std::mutex> lock(mutex);
        running = false;
        cv.notify_all();
    }
    if (worker_thread.joinable()) {
        worker_thread.join();
    }
}

uint64_t Time::get_global_tick() {
    std::lock_guard<std::mutex> lock(mutex);
    auto real_elapsed = RealClock::now() - simulation_start_time;
    simulation_time = std::chrono::duration_cast<SimDuration>(real_elapsed * simulation_speed);
    return simulation_time.count();
}

uint8_t Time::register_high_precision_alarm(uint32_t period_in_us, std::function<void()> func) {
    std::lock_guard<std::mutex> lock(mutex);
    uint8_t id = next_alarm_id++;

    Alarm alarm{
        SimDuration(period_in_us * 1000), // Convert microseconds to nanoseconds
        func,
        simulation_time + SimDuration(period_in_us * 1000),
        true,
    };

    alarms.emplace(id, std::move(alarm));
    cv.notify_all();
    return id;
}

bool Time::unregister_high_precision_alarm(uint8_t id) {
    std::lock_guard<std::mutex> lock(mutex);
    return alarms.erase(id) > 0;
}

uint8_t Time::register_low_precision_alarm(uint32_t period_in_ms, std::function<void()> func) {
    return register_high_precision_alarm(period_in_ms * 1000, func); //There is no hw limitation that does not allow this
}

bool Time::unregister_low_precision_alarm(uint8_t id) {
    return unregister_high_precision_alarm(id);
}

uint8_t Time::set_timeout(uint32_t milliseconds, std::function<void()> callback) {
    std::unique_lock<std::mutex> lock(mutex);
    uint8_t id = Time::next_alarm_id;
    lock.unlock();
    Time::register_low_precision_alarm(milliseconds, [&, id, callback](){
        callback();
        alarms[id].erase = true;
    });
    return id;
}

void Time::cancel_timeout(uint8_t id) {
    unregister_high_precision_alarm(id);
}

void Time::worker() {
    std::unique_lock<std::mutex> lock(mutex);

    while (running) {
        if (alarms.empty()) {
            cv.wait(lock, [] { return !alarms.empty() || !running; }); //No alarms
            continue;
        }

        auto now_real = RealClock::now();
        simulation_time = std::chrono::duration_cast<SimDuration>((now_real - simulation_start_time) * simulation_speed);

        // Find the soonest alarm
        auto next_alarm_iter = std::min_element(alarms.begin(), alarms.end(),
            [](const auto& a, const auto& b) {
                return a.second.next_trigger < b.second.next_trigger;
            });

        auto sim_now = simulation_time;
        auto sim_until = next_alarm_iter->second.next_trigger;
        auto sim_wait_duration = sim_until - sim_now;

        auto real_wait_duration = std::chrono::duration_cast<RealClock::duration>(sim_wait_duration / simulation_speed);

        if (real_wait_duration <= RealClock::duration::zero()) {
            // Real time cannot keep up with simulation time
            //TODO: ErrorHandler("Real time cannot keep up with simulation time.");
            break;
        }

        //Go to sleep until next alarm
        cv.wait_for(lock, real_wait_duration, [] { return !running; });

        if (!running) break;

        now_real = RealClock::now();
        simulation_time = std::chrono::duration_cast<SimDuration>((now_real - simulation_start_time) * simulation_speed);

        // Check if real time has fallen behind simulation time significantly
        auto expected_simulation_time = std::chrono::duration_cast<SimDuration>((now_real - simulation_start_time) * simulation_speed);
        lag = expected_simulation_time - simulation_time;
        if (lag > max_lag) {
            //ErrorHandler("Simulation time is significantly ahead of real time.");
            break;
        }

        for (auto it = alarms.begin(); it != alarms.end();) {
            if (it->second.active && it->second.next_trigger <= simulation_time) {
                auto alarm = it->second;
                lock.unlock();
                alarm.callback();
                lock.lock();
                it->second.next_trigger += alarm.period;
                if(alarm.erase){ 
                    it = alarms.erase(it);
                    continue;
                }
            }
            ++it;
        }
    }
}

Time::SimDuration Time::get_lag() {
    return lag;
}
