#pragma once
#include "LinearSensor.hpp"
#include "Control/Blocks/MovingAverage.hpp"
#ifdef FILTEREDLinearSensor_
template<class Type,size_t N>
class FilteredLinearSensor : public LinearSensor<Type>{
	MovingAverage<N>& filter;
public:

	FilteredLinearSensor(Pin& pin, Type slope, Type offset, Type* value, MovingAverage<N>& filter) : LinearSensor<Type>(pin, slope, offset, value), filter(filter){}
	FilteredLinearSensor(Pin& pin, Type slope, Type offset, Type& value, MovingAverage<N>& filter) : LinearSensor<Type>(pin, slope, offset, value), filter(filter){}
	void read(){
		float val = ADC::get_value(this->id);
		*this->value = filter.compute(this->slope * (Type) val + this->offset);
	}
};

//CTAD
#if __cpp_deduction_guides >= 201606
template<class Type,size_t N>
FilteredLinearSensor(Pin& pin, Type slope, Type offset, Type* value, MovingAverage<N>& filter)->FilteredLinearSensor<Type,N>;
template<class Type,size_t N>
FilteredLinearSensor(Pin& pin, Type slope, Type offset, Type& value, MovingAverage<N>& filter)->FilteredLinearSensor<Type,N>;
#endif
#endif