
/*
 * AnalogSensor.hpp
 *
 *  Created on: Nov 9, 2022
 *      Author: ricardo
 */

#pragma once
#include "ErrorHandler/ErrorHandler.hpp"
#include "HALAL/HALAL.hpp"

template <size_t SAMPLES>
class EncoderSensor {
    constexpr static uint32_t START_COUNTER{UINT32_MAX / 2};

    const double counter_distance_m;
    const int64_t delay_between_samples;

    uint8_t encoder_id;

    // We want to get the last buffer element and the midpoint, if the number of
    // elements is odd, these points and the present won't be evenly spaced
    // across time. The SAMPLES computation rounds it down to make it even
    RingBuffer<uint32_t, (SAMPLES / 2) * 2> past_delta_counters{};

    enum Direction : uint8_t { FORWARD = 0, BACKWARDS = 1 };

    Direction *direction;
    double *position;
    double *speed;
    double *acceleration;

   public:
    EncoderSensor(Pin &pin1, Pin &pin2, const double counter_distance_m,
                  Direction *direction, double *position, double *speed,
                  double *acceleration)
        : counter_distance_m(counter_distance_m),
          encoder_id(Encoder::inscribe(pin1, pin2)),
          direction(direction),
          position(position),
          speed(speed),
          acceleration(acceleration) {
        for (size_t i{0}; i < SAMPLES; ++i) past_delta_counters.push(0);
    }

    void turn_on() { Encoder::turn_on(encoder_id); }
    void turn_off() { Encoder::turn_off(encoder_id); }

    void reset() {
        Encoder::reset(encoder_id);
        for (size_t i{0}; i < SAMPLES; ++i) past_delta_counters.push_pop(0);
    }

    // must be called on equally spaced time periods
    void read() {
        uint32_t counter{Encoder::get_counter(encoder_id)};

        uint32_t delta_counter{counter - START_COUNTER};
        const uint32_t &previous_delta_counter{
            past_delta_counters[past_delta_counters.size() / 2 - 1]};
        const uint32_t &previous_previous_delta_counter{
            past_delta_counters[past_delta_counters.size() - 1]};

        *position = delta_counter * counter_distance_m;

        // https://en.wikipedia.org/wiki/Finite_difference_coefficient#Backward_finite_difference
        *speed = ((3.0 * delta_counter / 2.0) - (2.0 * previous_delta_counter) +
                  (previous_previous_delta_counter / 2.0)) *
                 counter_distance_m;

        *acceleration = (delta_counter - (2.0 * previous_delta_counter) +
                         previous_previous_delta_counter) *
                        counter_distance_m;

        *direction = Encoder::get_direction(encoder_id) ? FORWARD : BACKWARDS;

        past_delta_counters.push_pop(delta_counter);
    }
};
