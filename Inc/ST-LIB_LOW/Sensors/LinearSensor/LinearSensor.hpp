/*
 * LinearSensor.hpp
 *
 *  Created on: Nov 7, 2022
 *      Author: ricardo
 */

#pragma once
#include "HALAL/HALAL.hpp"
#include "ErrorHandler/ErrorHandler.hpp"
#include "Sensors/Sensor/Sensor.hpp"
#ifdef LinearSensor_
template <class Type>
    requires std::is_integral_v<Type> || std::is_floating_point_v<Type>
class LinearSensor {
   public:
    LinearSensor() = default;
    LinearSensor(Pin &pin, Type slope, Type offset, Type *value);
    LinearSensor(Pin &pin, Type slope, Type offset, Type &value);

    void read();
    uint8_t get_id();

    void set_offset(Type new_offset);
    Type get_offset();

    void set_gain(Type new_gain);
    Type get_gain();

    Type *get_value_pointer() const;

   protected:
    uint8_t id;
    Type slope;
    Type offset;
    Type *value;
};

template <class Type>
    requires std::is_integral_v<Type> || std::is_floating_point_v<Type>
LinearSensor<Type>::LinearSensor(Pin &pin, Type slope, Type offset, Type *value)
    : slope(slope), offset(offset), value(value) {
    id = ADC::inscribe(pin);

    Sensor::adc_id_list.push_back(id);
}

template <class Type>
    requires std::is_integral_v<Type> || std::is_floating_point_v<Type>
LinearSensor<Type>::LinearSensor(Pin &pin, Type slope, Type offset, Type &value)
    : LinearSensor::LinearSensor(pin, slope, offset, &value) {}

template <class Type>
    requires std::is_integral_v<Type> || std::is_floating_point_v<Type>
void LinearSensor<Type>::read() {
    float val = ADC::get_value(id);

    *value = slope * (Type)val + offset;
}

template <class Type>
    requires std::is_integral_v<Type> || std::is_floating_point_v<Type>
uint8_t LinearSensor<Type>::get_id() {
    return id;
}

template <class Type>
    requires std::is_integral_v<Type> || std::is_floating_point_v<Type>
Type LinearSensor<Type>::get_offset() {
    return offset;
}

template <class Type>
    requires std::is_integral_v<Type> || std::is_floating_point_v<Type>
Type LinearSensor<Type>::get_gain() {
    return slope;
}

template <class Type>
    requires std::is_integral_v<Type> || std::is_floating_point_v<Type>
void LinearSensor<Type>::set_offset(Type new_offset) {
    offset = new_offset;
}

template <class Type>
    requires std::is_integral_v<Type> || std::is_floating_point_v<Type>
void LinearSensor<Type>::set_gain(Type new_gain) {
    slope = new_gain;
}

template <class Type>
    requires std::is_integral_v<Type> || std::is_floating_point_v<Type>
Type *LinearSensor<Type>::get_value_pointer() const {
    return value;
}
#endif