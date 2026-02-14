#pragma once

#include <concepts>
#include <cstddef>
#include <cstdio>

#include "../ControlSystem.hpp"
#include "HALAL/Models/Concepts/Concepts.hpp"

template <size_t N> class MovingAverage : public ControlBlock<double, double> {
private:
    double buffer[N] = {0.0};
    uint32_t first = 0, last = -1;
    uint32_t counter = 0;
    double accumulator = 0.0;

public:
    MovingAverage() : ControlBlock<double, double>() { output_value = 0.0; }

    void execute() override {
        if (counter < N) {
            last++;
            buffer[last] = input_value;
            accumulator += input_value / N;
            counter++;
            return;
        }
        accumulator -= buffer[first] / N;
        first = (first + 1) % N;
        last = (last + 1) % N;
        buffer[last] = input_value;
        accumulator += buffer[last] / N;
        output_value = accumulator;
        return;
    }

    double compute(double input_v) {
        this->input(input_v);
        execute();
        return this->output_value;
    }

    void reset() {
        first = 0;
        last = -1;
        counter = 0;
        accumulator = 0;
        output_value = 0;
        for (double& e : buffer) {
            e = 0;
        }
    }
};

template <size_t N> class FloatMovingAverage : public ControlBlock<float, float> {
private:
    float buffer[N] = {0.0};
    uint32_t first = 0, last = -1;
    uint32_t counter = 0;
    float accumulator = 0.0;

public:
    FloatMovingAverage() : ControlBlock<float, float>() { output_value = 0.0; }

    void execute() override {
        if (counter < N) {
            last++;
            buffer[last] = input_value;
            accumulator += input_value / N;
            counter++;
            return;
        }
        accumulator -= buffer[first] / N;
        first = (first + 1) % N;
        last = (last + 1) % N;
        buffer[last] = input_value;
        accumulator += buffer[last] / N;
        output_value = accumulator;
        return;
    }

    float compute(float input_v) {
        this->input(input_v);
        execute();
        return this->output_value;
    }

    void reset() {
        first = 0;
        last = -1;
        counter = 0;
        accumulator = 0;
        output_value = 0;
        for (float& e : buffer) {
            e = 0;
        }
    }
};

/**
 * @brief version of moving average for integers. Shifts the values by DecimalBitCount before adding
 * to transform them into fixed arithmetic logic
 *
 * This version of the moving average implements the same logic as the MovingAverage class, but for
 * any kind of integral values. For any DecimalBitCount greater than one, it is recommended that the
 * OutputType is of greater size than the InputType to avoid overflows. The result is given in the
 * Fixed Point Arithmetic of DecimalBitCount number of decimal bits. That is to say, it is shifted
 * to the left by that amount. To translate back, shift the value to the right by DecimalBitCount
 * bits. Needless to say, that will remove the decimal data.
 */
template <std::integral InputType, std::integral OutputType, uint8_t DecimalBitCount, size_t N>
class IntegerMovingAverage : public ControlBlock<InputType, OutputType> {
private:
    FollowingUint<InputType>::Value buffer[N] = {0};
    uint32_t first = 0, last = -1;
    uint32_t counter = 0;
    FollowingUint<InputType>::Value accumulator = 0.0;

public:
    IntegerMovingAverage() : ControlBlock<InputType, OutputType>() { this->output_value = 0; }

    void execute() override {
        if (counter < N) {
            last++;
            buffer[last] = ((decltype(accumulator))this->input_value) << DecimalBitCount;
            accumulator += buffer[last] / N;
            counter++;
            return;
        }
        accumulator -= buffer[first] / N;
        first = (first + 1) % N;
        last = (last + 1) % N;
        buffer[last] = ((decltype(accumulator))this->input_value) << DecimalBitCount;
        accumulator += buffer[last] / N;
        this->output_value = (OutputType)accumulator;
        return;
    }

    OutputType compute(InputType input_v) {
        this->input(input_v);
        execute();
        return this->output_value;
    }

    void reset() {
        first = 0;
        last = -1;
        counter = 0;
        accumulator = 0;
        this->output_value = 0;
        for (InputType& e : buffer) {
            e = 0;
        }
    }
};
