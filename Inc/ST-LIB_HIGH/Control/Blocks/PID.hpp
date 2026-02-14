#include "../ControlSystem.hpp"
#include "Integrator.hpp"
#include "Derivator.hpp"

#pragma once

template <IntegratorType IntegratorMethod, FilterDerivatorType FilterDerivate, size_t... N>
class PID;

template <IntegratorType IntegratorMethod> class PID<IntegratorMethod, FilterDerivatorType::None> {
public:
    double kp, ki, kd;
    double error = 0;
    SimpleDerivator derivator;
    Integrator<IntegratorMethod> integrator;
    double output_value = 0.0;

public:
    PID(double kp, double ki, double kd, double period)
        : kp(kp), ki(ki), kd(kd), derivator(period), integrator(period, ki) {}
    double execute(double input_value) {
        derivator.input(input_value);
        derivator.execute();
        integrator.input(input_value);
        integrator.execute();
        error = input_value;
        this->output_value = kp * error + integrator.output_value + kd * derivator.output_value;
        return output_value;
    }
    void set_kp(double kp) {
        this->kp = kp;
        reset();
    }
    void set_ki(double ki) {
        this->ki = ki;
        reset();
    }
    void set_kd(double kd) {
        this->kd = kd;
        reset();
    }
    void reset() {
        integrator.reset();
        derivator.reset();
    }
};

template <IntegratorType IntegratorMethod, size_t N>
class PID<IntegratorMethod, FilterDerivatorType::Moving_Average, N>
    : public ControlBlock<double, double> {
public:
    double kp, ki, kd;
    double error = 0;
    SimpleDerivator derivator;
    Integrator<IntegratorMethod> integrator;
    MovingAverage<N> filter_derivative;

public:
    PID(double kp, double ki, double kd, double period)
        : kp(kp), ki(ki), kd(kd), derivator(period), integrator(period, ki) {}
    void execute() override {
        derivator.input(input_value);
        derivator.execute();
        integrator.input(input_value);
        integrator.execute();
        error = input_value;
        filter_derivative.input(kd * derivator.output_value);
        filter_derivative.execute();
        this->output_value = kp * error + integrator.output_value + filter_derivative.output_value;
    }
    void set_kp(double kp) {
        this->kp = kp;
        reset();
    }
    void set_ki(double ki) {
        this->ki = ki;
        reset();
    }
    void set_kd(double kd) {
        this->kd = kd;
        reset();
    }
    void reset() {
        integrator.reset();
        derivator.reset();
        filter_derivative.reset();
    }
};
