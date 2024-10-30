/*
 * test_time.cpp
 *
 *  Created on: 23 oct. 2024
 *      Author: Stefan Costea
 * 
 *  This example shows how the interface for Time HALALMock is used
 * 
 */
#pragma once
#include "Inc/HALALMock/Services/Time/Time.hpp"
#include <iostream>

#ifdef TESTING

uint8_t test_time(){
    // Start the simulation at 10x real speed
    Time::start(10);

    uint8_t alarm_id = Time::register_high_precision_alarm(1000000, []() {
        std::cout << "High precision alarm triggered every 1 second of simulation time." << std::endl;
    });
    
    uint8_t alarm2_id = Time::register_high_precision_alarm(2000000, []() {
        std::cout << "High precision alarm triggered every 2 second of simulation time." << std::endl;
    });
    
    uint8_t timeout_id = Time::set_timeout(1500, []() {
        std::cout << "Timeout triggered at 1.5 second of simulation time." << std::endl;
    });

    // Run for 2 seconds of real time (20 seconds simulation time)
    std::this_thread::sleep_for(std::chrono::seconds(5));

    Time::unregister_high_precision_alarm(alarm_id);
    Time::unregister_high_precision_alarm(alarm2_id);
    Time::stop();

    return 0;
}

#endif 