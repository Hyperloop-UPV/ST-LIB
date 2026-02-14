/*
 * NewEncoderSensor.hpp
 *
 *  Created on: 2 jan. 2026
 *      Author: Victor
 */
#pragma once

#include "HALAL/HALAL.hpp"
// #include "HALAL/Services/Encoder/NewEncoder.hpp"

namespace ST_LIB {

template <typename EncoderType, size_t SAMPLES> struct EncoderSensor {
    enum Direction : uint8_t { FORWARD = 0, BACKWARDS = 1 };

private:
    constexpr static int64_t START_COUNTER{UINT32_MAX / 2};

    const double counter_distance_m;
    const double sample_time_s;

    EncoderType& encoder;

    RingBuffer<int64_t, (SAMPLES / 2) * 2> past_delta_counters{};

    Direction* direction;
    double* position;
    double* speed;
    double* acceleration;

public:
    EncoderSensor(
        EncoderType& enc,
        const double counter_distance_m,
        const double sample_time_s,
        Direction* direction,
        double* position,
        double* speed,
        double* acceleration
    )
        : counter_distance_m(counter_distance_m), sample_time_s(sample_time_s), encoder(enc),
          direction(direction), position(position), speed(speed), acceleration(acceleration) {
        for (size_t i{0}; i < SAMPLES; i++)
            past_delta_counters.push(0);
    }

    void turn_on() { encoder.turn_on(); }
    void turn_off() { encoder.turn_off(); }

    void reset() {
        encoder.reset();
        for (size_t i{0}; i < SAMPLES; ++i)
            past_delta_counters.push_pop(0);
    }

    // must be called on equally spaced time periods
    void read() {
        uint32_t counter{encoder.get_counter()};

        int64_t delta_counter{(int64_t)counter - START_COUNTER};
        const int64_t& previous_delta_counter{
            past_delta_counters[past_delta_counters.size() / 2 - 1]
        };
        const int64_t& previous_previous_delta_counter{
            past_delta_counters[past_delta_counters.size() - 1]
        };

        *position = delta_counter * counter_distance_m;

        // https://en.wikipedia.org/wiki/Finite_difference_coefficient#Backward_finite_difference
        *speed = ((3.0 * delta_counter / 2.0) - (2.0 * previous_delta_counter) +
                  (previous_previous_delta_counter / 2.0)) *
                 counter_distance_m / (sample_time_s * past_delta_counters.size() / 2);

        *acceleration =
            (delta_counter - (2.0 * previous_delta_counter) + previous_previous_delta_counter) *
            counter_distance_m /
            ((sample_time_s * past_delta_counters.size() / 2) *
             (sample_time_s * past_delta_counters.size() / 2));

        *direction = encoder.get_direction() ? FORWARD : BACKWARDS;

        past_delta_counters.push_pop(delta_counter);
    }
};

} // namespace ST_LIB
