/*
 * LinearSensor.hpp
 *
 *  Created on: Nov 7, 2022
 *      Author: ricardo
 */

#pragma once
#include <cstdint>
#include <type_traits>

#include "HALAL/Services/ADC/NewADC.hpp"

template <class Type>
    requires std::is_integral_v<Type> || std::is_floating_point_v<Type>
class LinearSensor {
public:
    LinearSensor() = default;
    LinearSensor(
        ST_LIB::ADCDomain::Instance& adc,
        Type slope,
        Type offset,
        Type* value,
        float vref = 3.3f
    );
    LinearSensor(
        ST_LIB::ADCDomain::Instance& adc,
        Type slope,
        Type offset,
        Type& value,
        float vref = 3.3f
    );

    void read();

    void set_offset(Type new_offset);
    Type get_offset();

    void set_gain(Type new_gain);
    Type get_gain();

    Type* get_value_pointer() const;

protected:
    ST_LIB::ADCDomain::Instance* adc = nullptr;
    Type slope;
    Type offset;
    Type* value = nullptr;
    float vref = 3.3f;
};

template <class Type>
    requires std::is_integral_v<Type> || std::is_floating_point_v<Type>
LinearSensor<Type>::LinearSensor(
    ST_LIB::ADCDomain::Instance& adc,
    Type slope,
    Type offset,
    Type* value,
    float vref
)
    : adc(&adc), slope(slope), offset(offset), value(value), vref(vref) {}

template <class Type>
    requires std::is_integral_v<Type> || std::is_floating_point_v<Type>
LinearSensor<Type>::LinearSensor(
    ST_LIB::ADCDomain::Instance& adc,
    Type slope,
    Type offset,
    Type& value,
    float vref
)
    : LinearSensor::LinearSensor(adc, slope, offset, &value, vref) {}

template <class Type>
    requires std::is_integral_v<Type> || std::is_floating_point_v<Type>
void LinearSensor<Type>::read() {
    if (adc == nullptr || value == nullptr) {
        return;
    }
    const float raw = adc->get_raw();
    const float val = adc->get_value_from_raw(raw, vref);

    *value = slope * (Type)val + offset;
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
Type* LinearSensor<Type>::get_value_pointer() const {
    return value;
}
