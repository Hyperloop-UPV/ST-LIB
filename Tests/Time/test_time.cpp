/*
 * test_time.cpp
 *
 *  Created on: 23 oct. 2024
 *      Author: Stefan Costea
 * 
 *  This example shows how the interface for Time HALALMock is used
 * 
 */

#include "HALALMock/Services/Time/Time.hpp"
#include <gtest/gtest.h>
#include <iostream>

std::atomic<int>high_precision_alarm_count{0};
std::atomic<bool>timeout_triggered{false};


TEST(Time,High_Precision_Alarm){
    // Start the simulation at 10x real speed
    Time::start(10);

    uint8_t alarm_id= Time::register_high_precision_alarm(1000000,[&](){
        high_precision_alarm_count++;
    });

    // Sleep for 2 seconds of real time (20 seconds simulation time)
    std::this_thread::sleep_for(std::chrono::seconds(2));

    Time::unregister_high_precision_alarm(alarm_id);

    Time::stop();

    // Check if the alarm was triggered 20 times
    EXPECT_EQ(high_precision_alarm_count, 20);
}

TEST(Time,Multiple_Alarms){
    // Start the simulation at 10x real speed
    Time::start(10);

    uint8_t alarm_id_0= Time::register_high_precision_alarm(1000000,[&](){
        high_precision_alarm_count++;
    });

    uint8_t alarm_id_1= Time::register_mid_precision_alarm(1000000,[&](){
        high_precision_alarm_count--;
    });

    // Sleep for 2 seconds of real time (20 seconds simulation time)
    std::this_thread::sleep_for(std::chrono::seconds(2));

    Time::unregister_high_precision_alarm(alarm_id_0);
    Time::unregister_mid_precision_alarm(alarm_id_1);

    Time::stop();

    // Check if the alarm was triggered 20 times
    EXPECT_EQ(high_precision_alarm_count, 0);
}

TEST(Time,Timeout){
    // Start the simulation at 10x real speed
    Time::start(10);

    // gets the global tick before the timeout
    uint64_t start_time=Time::get_global_tick();
    std::atomic<uint64_t>end_time{0};

    uint8_t timeout_id=Time::set_timeout(1500,[&](){
        timeout_triggered=true;
        // gets the global tick after the timeout
        end_time=Time::get_global_tick();

    });

    std::this_thread::sleep_for(std::chrono::seconds(2));

    Time::stop();

    EXPECT_TRUE(timeout_triggered);
    EXPECT_NE(end_time, 0); 
    EXPECT_GE(end_time - start_time, 1490000000); // upper bound 
    EXPECT_LE(end_time - start_time, 1510000000); // lower bound 
}

