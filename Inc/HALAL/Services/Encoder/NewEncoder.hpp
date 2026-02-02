/*
 * NewEncoder.hpp
 *
 *  Created on: 2 jan. 2026
 *      Author: Victor
 */
#pragma once

#include "HALAL/Services/Time/TimerWrapper.hpp"

namespace ST_LIB {

template<const TimerDomain::Timer &dev>
struct Encoder {
    static_assert(dev.e.pin_count == 2, "Encoder must have exactly 2 encoder pins, as it uses the whole timer");
    static_assert(dev.e.pins[0].af == TimerAF::Encoder, "Pin 0 must be declared as encoder");
    static_assert(dev.e.pins[1].af == TimerAF::Encoder, "Pin 1 must be declared as encoder");
    static_assert(dev.e.pins[0].channel != dev.e.pins[1].channel, "Pins must be of different channels");

    static TimerWrapper<dev> *timer;
    static void init(TimerWrapper<dev> *timer, uint16_t prescaler, uint32_t period);

    static void turn_on();

    static void turn_off();

    static void reset();

    static uint32_t get_counter();

    static bool get_direction();

    static uint32_t get_initial_counter_value();

    static int64_t get_delta_clock(uint64_t clock_time, uint64_t last_clock_time);
};

}