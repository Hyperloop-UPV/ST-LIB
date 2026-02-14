#pragma once
#include "../ControlBlock.hpp"
#include "MeanCalculator.hpp"
#include "Sensors/LinearSensor/LinearSensor.hpp"
template <class Type, size_t Order> class Zeroing : public ControlBlock<Type, Type> {
public:
    LinearSensor<Type>& sensor;
    MeanCalculator<Order> mean_calculator;
    Type max_value_offset;
    bool has_max_value = false;
    Zeroing(LinearSensor<Type>& sensor) : sensor(sensor), mean_calculator(){};
    Zeroing(LinearSensor<Type>& sensor, Type max_value_offset)
        : mean_calculator(), sensor(sensor), max_value_offset(max_value_offset),
          has_max_value(true){};
    void execute() {
        while (mean_calculator.output_value == 0) {
            sensor.read();
            mean_calculator.input(*sensor.get_value_pointer());
            mean_calculator.execute();
        }
        if (has_max_value && abs(mean_calculator.output_value) > max_value_offset) {
            ErrorHandler("Zeroing offset is calculated to be above specified maximum");
        } else
            sensor.set_offset(sensor.get_offset() - mean_calculator.output_value);
        mean_calculator.reset();
    }
};
