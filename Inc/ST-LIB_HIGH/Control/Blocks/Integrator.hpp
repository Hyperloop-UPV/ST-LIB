#pragma once
#include "../ControlBlock.hpp"

#pragma once

enum class IntegratorType { ForwardEuler, BackwardEuler, Trapezoidal };

template <IntegratorType T> class Integrator;

template <IntegratorType T> class FloatIntegrator;

template <> class Integrator<IntegratorType::Trapezoidal> : public ControlBlock<double, double> {
public:
    static constexpr int N = 2;
    double period;
    double buffer[N] = {0.0};
    int index = 0;
    double integral = 0.0;
    double ki;
    bool first_execution = true;

    Integrator() = default;
    Integrator(double period, double ki)
        : ControlBlock<double, double>(0.0), period(period), ki(ki) {
        output_value = 0.0;
    }
    void execute() override {
        buffer[index] = input_value;
        if (first_execution) {
            first_execution = false;
            index++;
            index %= N;
            output_value = 0.0;
            return;
        }
        integral += ki * period * (buffer[index] + buffer[((index - 1) % (N) + (N)) % (N)]) / 2.0;
        output_value = integral;
        index++;
        index %= N;
        return;
    }
    void reset() {
        output_value = 0;
        first_execution = true;
        integral = 0.0;
        index = 0;
        for (int i = 0; i < N; i++) {
            buffer[i] = 0.0;
        }
    }
};

template <> class FloatIntegrator<IntegratorType::Trapezoidal> : public ControlBlock<float, float> {
public:
    static constexpr int N = 2;
    float period;
    float buffer[N] = {0.0};
    int index = 0;
    float integral = 0.0;
    float ki;
    bool first_execution = true;

    FloatIntegrator() = default;
    FloatIntegrator(float period, float ki)
        : ControlBlock<float, float>(0.0), period(period), ki(ki) {
        output_value = 0.0;
    }
    void execute() override {
        buffer[index] = input_value;
        if (first_execution) {
            first_execution = false;
            index++;
            index %= N;
            output_value = 0.0;
            return;
        }
        integral += ki * period * (buffer[index] + buffer[((index - 1) % (N) + (N)) % (N)]) / 2.0;
        output_value = integral;
        index++;
        index %= N;
        return;
    }
    void reset() {
        output_value = 0;
        first_execution = true;
        integral = 0.0;
        index = 0;
        for (int i = 0; i < N; i++) {
            buffer[i] = 0.0;
        }
    }
};

template <> class Integrator<IntegratorType::ForwardEuler> : public ControlBlock<double, double> {
public:
    static constexpr int N = 2;
    double period;
    double buffer[N] = {0.0};
    int index = 0;
    double integral = 0.0;
    double ki;
    bool first_execution = true;

public:
    Integrator(double period, double ki)
        : ControlBlock<double, double>(0.0), period(period), ki(ki) {
        this->output_value = 0.0;
    }
    void execute() override {
        buffer[index] = input_value;
        if (first_execution) {
            first_execution = false;
            index++;
            index %= N;
            output_value = 0.0;
            return;
        }
        integral += ki * period * buffer[((index - 1) % (N) + (N)) % (N)];
        output_value = integral;
        index++;
        index %= N;
        return;
    }
    void reset() {
        first_execution = true;
        integral = 0.0;
        index = 0;
        for (int i = 0; i < N; i++) {
            buffer[i] = 0.0;
        }
    }
};

template <> class Integrator<IntegratorType::BackwardEuler> : public ControlBlock<double, double> {
public:
    static constexpr int N = 2;
    double period;
    double buffer[N] = {0.0};
    int index = 0;
    double integral = 0.0;
    double ki;
    bool first_execution = true;

public:
    Integrator(double period, double ki)
        : ControlBlock<double, double>(0.0), period(period), ki(ki) {
        output_value = 0.0;
    }
    void execute() override {
        buffer[index] = input_value;
        if (first_execution) {
            first_execution = false;
            index++;
            index %= N;
            output_value = 0.0;
            return;
        }
        integral += ki * period * buffer[index];
        output_value = integral;
        index++;
        index %= N;
        return;
    }
    void reset() {
        first_execution = true;
        integral = 0.0;
        index = 0;
        for (int i = 0; i < N; i++) {
            buffer[i] = 0.0;
        }
    }
};
