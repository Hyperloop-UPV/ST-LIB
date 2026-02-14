#pragma once

#include "../ControlBlock.hpp"
#include "Integrator.hpp"

template <IntegratorType IntegratorMethod> class PI : public ControlBlock<double, double> {
public:
    double kp;
    double error;
    Integrator<IntegratorMethod> integrator;

public:
    PI() = default;
    PI(double kp, double ki, double period) : kp(kp), integrator(period, ki) {}
    void execute() override {
        integrator.input(this->input_value);
        integrator.execute();
        error = this->input_value;
        this->output_value = kp * error + integrator.output_value;
    }
    void set_kp(double kp) { this->kp = kp; }
    void set_ki(double ki) {
        integrator.ki = ki;
        integrator.reset();
    }
    void reset() { integrator.reset(); }
};
