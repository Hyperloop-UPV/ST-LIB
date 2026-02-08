#pragma once
#include <cstddef>
#include <cstdint>

#include "Control/Blocks/MovingAverage.hpp"
#include "HALAL/Services/ADC/NewADC.hpp"
#include "Sensors/AnalogUtils.hpp"


template<size_t N>
class PT100{
public:
	static constexpr float k = 841.836735;
	static constexpr float offset = -492.204082;
	MovingAverage<N>* filter = nullptr;

	PT100(ST_LIB::ADCDomain::Instance& adc, float* value, MovingAverage<N>& filter);
	PT100(ST_LIB::ADCDomain::Instance& adc, float* value);

	PT100(ST_LIB::ADCDomain::Instance& adc, float& value, MovingAverage<N>& filter);
	PT100(ST_LIB::ADCDomain::Instance& adc, float& value);

	void read();
protected:
	ST_LIB::ADCDomain::Instance* adc = nullptr;
	float* value = nullptr;
};

template<size_t N>
PT100<N>::PT100(ST_LIB::ADCDomain::Instance& adc, float* value, MovingAverage<N>& filter)
	: adc(&adc), value(value), filter(&filter){}

template<size_t N>
PT100<N>::PT100(ST_LIB::ADCDomain::Instance& adc, float& value, MovingAverage<N>& filter)
	: adc(&adc), value(&value), filter(&filter){}

template<size_t N>
PT100<N>::PT100(ST_LIB::ADCDomain::Instance& adc, float* value)
	: adc(&adc), value(value){}

template<size_t N>
PT100<N>::PT100(ST_LIB::ADCDomain::Instance& adc, float& value)
	: adc(&adc), value(&value){}

template<size_t N>
void PT100<N>::read(){
	if(adc == nullptr || value == nullptr){
		return;
	}
	const uint32_t raw = adc->read_raw();
	const float val = ST_LIB::adc_raw_to_voltage(raw, adc->resolution);
	if(filter != nullptr){
		filter->input(k/val + offset);
		filter->execute();
		*value = filter->output_value;
	}else *value = k/val + offset;
}
