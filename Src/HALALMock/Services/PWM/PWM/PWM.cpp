#include "HALALMock/Services/PWM/PWM/PWM.hpp"
PWM::PWM(Pin& pin) {
	/*
	HALALMock only tests the logical interface of the PWM works
    
    If the actual code doesn't work it might be related to
	the timers and the channels
	*/
	if (not available_pwm.contains(pin)) {
		ErrorHandler("Pin %s is not registered as an available PWM", pin.to_string());
		return;
	}
	
	EmulatedPin &pin_data = SharedMemory::get_pin(pin);
	if(pin_data.type == PinType::NOT_USED){
		pin_data.type = PinType::PWM;
		//let's point our class variables to the variables from PinModel
		duty_cycle = &(pin_data.PinData.pwm.duty_cycle);
		frequency = &(pin_data.PinData.pwm.frequency);
		is_on = &(pin_data.PinData.pwm.is_on);
		dead_time_ns = &(pin_data.PinData.pwm.dead_time_ns);
		
		//default values
		*duty_cycle = 0.0f;
		*is_on = false;
		*frequency = 0;
		*dead_time_ns = std::chrono::nanoseconds(0).count();
	}else{
		ErrorHandler("Pin %s is being used already",pin.to_string());
	}
}

void PWM::turn_on() {
		*is_on = true;
}

void PWM::turn_off() {
  *is_on = false;
}

void PWM::set_duty_cycle(float dc){
	*duty_cycle = dc;
}

void PWM::set_frequency(uint32_t freq) {
	*frequency = freq;
}

uint32_t PWM::get_frequency() {
	return *frequency;
}

float PWM::get_duty_cycle(){
	return *duty_cycle;
}
void PWM::set_dead_time(std::chrono::nanoseconds dead_t_ns)
{
	if(*is_on)
		ErrorHandler("%s","This function can not be called if the PWM is on");
	else
		*dead_time_ns=dead_t_ns.count();	
}
