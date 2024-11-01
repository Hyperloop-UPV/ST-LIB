/*
 * PhasedPWM.cpp
 *
 *  Created on: Feb 27, 2023
 *      Author: aleja
 */
/*
#include "HALALMock/Services/PWM/PhasedPWM/PhasedPWM.hpp"
#include "HALALMock/Models/TimerPeripheral/TimerPeripheral.hpp"
*/
/**
 * The function initializes a PhasedPWM object with a given pin and sets its duty cycle and phase to 0.
 * 
 * @param pin The pin to which the PhasedPWM object is being attached.
 */
/*PhasedPWM::PhasedPWM(Pin& pin) {
	if (not TimerPeripheral::available_pwm.contains(pin)) {
		ErrorHandler("Pin %s is not registered as an available PWM", pin.to_string());
		return;
	}

	TimerPeripheral& timer = TimerPeripheral::available_pwm.at(pin).first;
	TimerPeripheral::PWMData& pwm_data = TimerPeripheral::available_pwm.at(pin).second;

	if (pwm_data.mode != TimerPeripheral::PWM_MODE::PHASED) {
		ErrorHandler("Pin %s is not registered as a PHASED PWM", pin.to_string());
	}

	peripheral = &timer;
	channel = pwm_data.channel;

	Pin::inscribe(pin, TIMER_ALTERNATE_FUNCTION);
	timer.init_data.pwm_channels.push_back(pwm_data);

	duty_cycle = 0;
	raw_phase = 0;
	is_initialized = true;
}*/

/**
 * This function sets the duty cycle of a PWM signal and calculates the raw duty and phase values based
 * on the input duty cycle and phase.
 * 
 * @param duty_cycle The duty cycle is a value between 0 and 100 that represents the percentage of time
 * that the PWM signal is high compared to the total period of the signal.
 */
/*void PhasedPWM::set_duty_cycle(float duty_cycle) {
	this->duty_cycle = duty_cycle;
	if(raw_phase > 100.0){
		duty_cycle = 100.0 - duty_cycle;
		raw_phase = raw_phase - 100.0;
		__STLIB_TIM_SET_MODE(peripheral->handle, channel, STLIB_TIMER_CCMR_PWM_MODE_1)
	}else{
		__STLIB_TIM_SET_MODE(peripheral->handle, channel, STLIB_TIMER_CCMR_PWM_MODE_2)
	}
	uint32_t arr = peripheral->handle->Instance->ARR;
	float start_high = arr*(50.0 - duty_cycle)/50.0;
	float end_high = arr*(100.0 - duty_cycle)/50.0 + 1;
	if(start_high < 0){start_high = 0;}
	if(end_high > arr){end_high = arr;}
	float max_range = duty_cycle > 50.0 ? 100 - duty_cycle : duty_cycle;
	start_high = start_high + arr * max_range * raw_phase / 5000.0;
	end_high = end_high - arr * max_range * raw_phase / 5000.0;


	peripheral->handle->Instance->CR2 &= ~TIM_CR2_CCPC;
	peripheral->handle->Instance->CR2 &= ~TIM_CR2_CCUS;

	__HAL_TIM_SET_COMPARE(peripheral->handle, channel, start_high);

	if (channel % 8 == 0) {
		__HAL_TIM_SET_COMPARE(peripheral->handle, channel + 4, end_high);
		peripheral->handle->Instance->EGR |= TIM_EGR_UG;
		peripheral->handle->Instance->CR2 |= TIM_CR2_CCPC;
		peripheral->handle->Instance->CR2 |= TIM_CR2_CCUS;
		TIM_CCxChannelCmd(peripheral->handle->Instance, channel, TIM_CCx_ENABLE);
		TIM_CCxChannelCmd(peripheral->handle->Instance, channel + 4, TIM_CCx_ENABLE);
		__HAL_TIM_MOE_ENABLE(peripheral->handle);
	} else {
		__HAL_TIM_SET_COMPARE(peripheral->handle, channel - 4, end_high);
		peripheral->handle->Instance->EGR |= TIM_EGR_UG;
		peripheral->handle->Instance->CR2 |= TIM_CR2_CCPC;
		peripheral->handle->Instance->CR2 |= TIM_CR2_CCUS;
		TIM_CCxChannelCmd(peripheral->handle->Instance, channel, TIM_CCx_ENABLE);
		TIM_CCxChannelCmd(peripheral->handle->Instance, channel - 4, TIM_CCx_ENABLE);
		__HAL_TIM_MOE_ENABLE(peripheral->handle);
	}
}*/

/**
 * This function sets the frequency of a PWM signal using a timer in a microcontroller.
 * 
 * @param frequency The desired frequency of the PWM signal in Hertz (Hz).
 */
/*void PhasedPWM::set_frequency(uint32_t frequency) {
	TIM_TypeDef& timer = *peripheral->handle->Instance;
	timer.ARR = (HAL_RCC_GetPCLK1Freq()*2 / (timer.PSC+1)) / 2 / frequency;
	this->frequency = frequency;
	set_duty_cycle(duty_cycle);
}*/

/**
 * This function sets the phase of a PhasedPWM object and updates the duty cycle accordingly.
 * 
 * @param phase The "phase" parameter is a floating-point value that represents the phase shift of a
 * PWM signal. In other words, it determines the timing offset of the PWM waveform relative to its center.
 * Only works with duty cycle = 50 for now.
 */
/*void PhasedPWM::set_phase(float phase_in_deg) {
	if(duty_cycle == 50.0){
	this->raw_phase = phase_in_deg*(200.0/360.0);
	}else{
		//TODO
	}
	set_duty_cycle(duty_cycle);
}

void PhasedPWM::set_raw_phase(float raw_phase){
	this->raw_phase = raw_phase;
	set_duty_cycle(duty_cycle);

}

float PhasedPWM::get_phase()const{
	return raw_phase*360/200;
}
*/