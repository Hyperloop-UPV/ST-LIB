#pragma once

#include "Control/Blocks/MovingAverage.hpp"
#include "LinearSensor.hpp"
#include "Sensors/AnalogUtils.hpp"

template <class Type, size_t N>
class FilteredLinearSensor : public LinearSensor<Type> {
  MovingAverage<N> &filter;

public:
  FilteredLinearSensor(ST_LIB::ADCDomain::Instance &adc, Type slope,
                       Type offset, Type *value, MovingAverage<N> &filter)
      : LinearSensor<Type>(adc, slope, offset, value), filter(filter) {}

  FilteredLinearSensor(ST_LIB::ADCDomain::Instance &adc, Type slope,
                       Type offset, Type &value, MovingAverage<N> &filter)
      : LinearSensor<Type>(adc, slope, offset, value), filter(filter) {}

  void read() {
    if (this->adc == nullptr || this->value == nullptr) {
      return;
    }
    const uint32_t raw = this->adc->read_raw();
    const float val =
        ST_LIB::adc_raw_to_voltage(raw, this->adc->resolution, this->vref);
    *this->value =
        filter.compute(this->slope * static_cast<Type>(val) + this->offset);
  }
};

// CTAD
#if __cpp_deduction_guides >= 201606
template <class Type, size_t N>
FilteredLinearSensor(ST_LIB::ADCDomain::Instance &adc, Type slope, Type offset,
                     Type *value, MovingAverage<N> &filter)
    -> FilteredLinearSensor<Type, N>;
template <class Type, size_t N>
FilteredLinearSensor(ST_LIB::ADCDomain::Instance &adc, Type slope, Type offset,
                     Type &value, MovingAverage<N> &filter)
    -> FilteredLinearSensor<Type, N>;
#endif
