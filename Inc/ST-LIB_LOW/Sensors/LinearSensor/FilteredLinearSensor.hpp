#pragma once

#include "Control/Blocks/MovingAverage.hpp"
#include "LinearSensor.hpp"

template <class Type, size_t N> class FilteredLinearSensor : public LinearSensor<Type> {
    MovingAverage<N>& filter;

public:
    FilteredLinearSensor(
        ST_LIB::ADCDomain::Instance& adc,
        Type slope,
        Type offset,
        Type* value,
        MovingAverage<N>& filter
    )
        : LinearSensor<Type>(adc, slope, offset, value), filter(filter) {}

    FilteredLinearSensor(
        ST_LIB::ADCDomain::Instance& adc,
        Type slope,
        Type offset,
        Type& value,
        MovingAverage<N>& filter
    )
        : LinearSensor<Type>(adc, slope, offset, value), filter(filter) {}

    void read() {
        if (this->adc == nullptr || this->value == nullptr) {
            return;
        }
        const float raw = this->adc->get_raw();
        const float val = this->adc->get_value_from_raw(raw, this->vref);
        *this->value = filter.compute(this->slope * static_cast<Type>(val) + this->offset);
    }
};

// CTAD
#if __cpp_deduction_guides >= 201606
template <class Type, size_t N>
FilteredLinearSensor(
    ST_LIB::ADCDomain::Instance& adc,
    Type slope,
    Type offset,
    Type* value,
    MovingAverage<N>& filter
) -> FilteredLinearSensor<Type, N>;
template <class Type, size_t N>
FilteredLinearSensor(
    ST_LIB::ADCDomain::Instance& adc,
    Type slope,
    Type offset,
    Type& value,
    MovingAverage<N>& filter
) -> FilteredLinearSensor<Type, N>;
#endif
