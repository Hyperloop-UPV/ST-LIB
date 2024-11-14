/*
 * find_max_simulation_speed_example.cpp
 *
 *  Created on: 23 oct. 2024
 *      Author: Stefan Costea
 *
 *  This example shows a real use of Time HALALMock, in this case this test uses Golden section search
 *  to find the maximum possible simulation speed that your machine can handle before reaching a significant
 *  amount of lag, defined by the threshold "max_lag".
 *
 */
#pragma once

#include "Inc/HALALMock/Services/Time/Time.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>

#ifdef TESTING

double measure_lag(double simulation_speed, int test_duration_seconds) {

    Time::start(simulation_speed);

    uint8_t alarm_id = Time::register_high_precision_alarm(1000000, []() {
        // Minimal callback; do nothing
    });

    std::this_thread::sleep_for(std::chrono::seconds(test_duration_seconds));

    uint64_t lag = Time::get_lag().count();

    Time::unregister_high_precision_alarm(alarm_id);
    Time::stop();

    return static_cast<double>(lag);
}

//lag in ns
double find_max_simulation_speed(double max_lag) {
    //test config
    double min_speed = 1.0, max_speed = 1000.0, tolerance = 1;
    const int test_duration = 2;

    //Golden ratio constants
    const double phi = (1 + std::sqrt(5)) / 2;
    const double resphi = 2 - phi;
    double c = max_speed - resphi * (max_speed - min_speed);
    double d = min_speed + resphi * (max_speed - min_speed);

    double lag_c = measure_lag(c, test_duration);
    double lag_d = measure_lag(d, test_duration);
    double best_speed = min_speed;

    while ((max_speed - min_speed) > tolerance) {
        std::cout << "min speed: "<<min_speed << " max_speed: " << max_speed << std::endl;
        if (lag_c <= max_lag) {
            min_speed = c;
            best_speed = c;
        } else {
            max_speed = c;
        }
        c = max_speed - resphi * (max_speed - min_speed);
        lag_c = measure_lag(c, test_duration);

        if (lag_d <= max_lag) {
            min_speed = d;
            best_speed = d;
        } else {
            max_speed = d;
        }
        d = min_speed + resphi * (max_speed - min_speed);
        lag_d = measure_lag(d, test_duration);
    }

    std::cout << "Maximum simulation speed under max lag " << max_lag << " ns: " << best_speed << "x" << std::endl;

    return best_speed;
}


#endif