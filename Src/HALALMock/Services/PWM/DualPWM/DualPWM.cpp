/*
 * DualPWM.cpp
 *
 *  Created on: Feb 27, 2023
 *      Author: aleja
 */

#include "HALALMock/Services/PWM/DualPWM/DualPWM.hpp"

DualPWM::DualPWM(Pin& pin, Pin& pin_negated):pin_positive(SharedMemory::get_pin(pin)), pin_negative(SharedMemory::get_pin(pin_negated)){

	if (not available_dual_pwms.contains({pin, pin_negated})) {
		ErrorHandler("Pins %s and %s are not registered as an available Dual PWM", pin.to_string(), pin_negated.to_string());
		return;
	}

	if(pin_positive.type==PinType::NOT_USED && pin_negative.type==PinType::NOT_USED){
		pin_positive.type=PinType::DualPWM;
		// for common values between positive and negative pin, class variables
		// point to the positive pin memory location, then the negative pin 
		// copies the value
		this->duty_cycle=&(pin_positive.PinData.dual_pwm.duty_cycle);
		this->frequency=&(pin_positive.PinData.dual_pwm.frequency);
		positive_is_on=&(pin_positive.PinData.dual_pwm.is_on);
		this->dead_time_ns=&(pin_positive.PinData.dual_pwm.dead_time_ns);

		pin_negative.type=PinType::DualPWM;
		negative_is_on=&(pin_negative.PinData.dual_pwm.is_on);

		//default values

		*(this->duty_cycle)=0.0f;
		pin_negative.PinData.dual_pwm.duty_cycle=*(this->duty_cycle);
		*(this->frequency)=0;
		pin_negative.PinData.dual_pwm.frequency=*(this->frequency);
		*positive_is_on=false;
		*negative_is_on=false;
		*(this->dead_time_ns)=std::chrono::nanoseconds(0);
		pin_negative.PinData.dual_pwm.dead_time_ns=*(this->dead_time_ns);

	}else{
		ErrorHandler("Pin %s or %s is already in use", pin.to_string(), pin_negated.to_string());
		return;
	}
}


void DualPWM::turn_on() {
	if(ErrorHandlerModel::error_triggered==0){
	*positive_is_on=true;
	*negative_is_on=true;
	}
}

void DualPWM::turn_off() {
	if(ErrorHandlerModel::error_triggered==0){
	*positive_is_on=false;
	*negative_is_on=false;
	}
}

void DualPWM::turn_on_positive() {
	if(ErrorHandlerModel::error_triggered==0){
	*positive_is_on=true;
	}
}

void DualPWM::turn_on_negated() {
	if(ErrorHandlerModel::error_triggered==0){
	*negative_is_on=true;
	}
}

void DualPWM::turn_off_positive() {
	if(ErrorHandlerModel::error_triggered==0){
  	*positive_is_on=false;
	}
}

void DualPWM::turn_off_negated() {
	if(ErrorHandlerModel::error_triggered==0){
	*negative_is_on=false;
	}
}

void DualPWM::set_duty_cycle(float duty_cycle){
	if(ErrorHandlerModel::error_triggered==0){
	*(this->duty_cycle) = duty_cycle;
	pin_negative.PinData.dual_pwm.duty_cycle=*(this->duty_cycle);
	}
}

void DualPWM::set_frequency(uint32_t freq_in_hz){
	if(ErrorHandlerModel::error_triggered==0){
  	*(this->frequency) = freq_in_hz;
	pin_negative.PinData.dual_pwm.frequency=*(this->frequency);
	}
}

uint32_t DualPWM::get_frequency()const{
  return *(this->frequency);
}

float DualPWM::get_duty_cycle()const{
  return *(this->duty_cycle);
}

void DualPWM::set_dead_time(std::chrono::nanoseconds dead_time_ns)
{
	if(ErrorHandlerModel::error_triggered==0){
	if(*positive_is_on || *negative_is_on)
		ErrorHandler("%s","This function can not be called if the PWM is on");
	else{
		*(this->dead_time_ns)=dead_time_ns;
		pin_negative.PinData.dual_pwm.dead_time_ns=*(this->dead_time_ns);
	}
	}
}

