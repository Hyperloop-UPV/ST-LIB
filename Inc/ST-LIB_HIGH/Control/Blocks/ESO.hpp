/*
 * ESO.hpp
 *
 *  Created on: 25 dec. 2025
 *      Author: Boris
 */

#ifndef ESO_HPP
#define ESO_HPP

#include "../ControlBlock.hpp"
#include "C++Utilities/CppUtils.hpp"
#include "HALAL/Models/Concepts/Concepts.hpp"

template <ControlSignal T, typename Config>
concept ESOConfig = requires {
    { Config::b0 } -> std::convertible_to<T>;
    { Config::omega_o } -> std::convertible_to<T>;
    { Config::period } -> std::convertible_to<T>;
};


template <ControlSignal T, ESOConfig<T> Config, int Order> requires (Order == 2 || Order == 3) // Doesn't support higher orders, can be extended with a general vector and matrix implementation
class ESO;

/**
 * @brief 2nd Order ESO for 1st Order Plant. Uses casting to T to allow for float/double/q31 etc.
 * 
 * - z1: Output (y)
 * 
 * - z2: Total Disturbance
 * 
 * In the Config class, define:
 * 
 * - static constexpr T b0: Estimated control gain
 * 
 * - static constexpr T omega_o: Observer bandwidth
 * 
 * - static constexpr T period: Execution period
 */
template <ControlSignal T, ESOConfig<T> Config>
class ESO<T, Config, 2> : public ControlBlock<T, T> {
public:
    static constexpr T b0 = Config::b0;
    static constexpr T omega_o = Config::omega_o;
    static constexpr T period = Config::period;

    static constexpr T beta1 = (T)2.0 * omega_o;
    static constexpr T beta2 = omega_o * omega_o;

    T z1 = 0; // Estimate of y
    T z2 = 0; // Estimate of disturbance

    T measured_value = 0;

    ESO() : ControlBlock<T, T>((T)0) {}

    void input_measurement(T measurement) {
        this->measured_value = measurement;
    }

    void execute() override {
        T u = this->input_value;
        T y = this->measured_value;
        T error = z1 - y;
        
        T dz1 = z2 - beta1 * error + b0 * u;
        T dz2 = (T)-1.0 * beta2 * error;

        z1 += dz1 * period;
        z2 += dz2 * period;
        
        this->output_value = z2;
    }

    void reset() {
        z1 = 0;
        z2 = 0;
        measured_value = 0;
        this->output_value = 0;
    }
};

/**
 * @brief 3rd Order ESO for 2nd Order Plants. Uses casting to T to allow for float/double/q31 etc.
 * 
 * - z1: Output (y)
 * 
 * - z2: Derivative (y_dot)
 * 
 * - z3: Total Disturbance
 * 
 * In the Config class, define:
 * 
 * - static constexpr T b0: Estimated control gain
 * 
 * - static constexpr T omega_o: Observer bandwidth
 * 
 * - static constexpr T period: Execution period
 */
template <ControlSignal T, ESOConfig<T> Config>
class ESO<T, Config, 3> : public ControlBlock<T, T> {
public:
    static constexpr T b0 = Config::b0;
    static constexpr T omega_o = Config::omega_o;
    static constexpr T period = Config::period;

    static constexpr T beta1 = (T)3.0 * omega_o;
    static constexpr T beta2 = (T)3.0 * omega_o * omega_o;
    static constexpr T beta3 = omega_o * omega_o * omega_o;

    T z1 = 0; // Estimate of y
    T z2 = 0; // Estimate of y_dot
    T z3 = 0; // Estimate of disturbance

    T measured_value = 0;

    ESO() : ControlBlock<T, T>((T)0) {}

    void input_measurement(T measurement) {
        this->measured_value = measurement;
    }

    void execute() override {
        T u = this->input_value;
        T y = this->measured_value;
        T error = z1 - y;
        
        T dz1 = z2 - beta1 * error;
        T dz2 = z3 - beta2 * error + b0 * u;
        T dz3 = (T)-1.0 * beta3 * error;

        z1 += dz1 * period;
        z2 += dz2 * period;
        z3 += dz3 * period;

        this->output_value = z3;
    }

    void reset() {
        z1 = 0;
        z2 = 0;
        z3 = 0;
        measured_value = 0;
        this->output_value = 0;
    }
};

#endif /* ESO_HPP */