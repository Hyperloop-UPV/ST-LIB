/*
 * DualPWMInstance.hpp
 *
 *  Created on: Feb 27, 2023
 *      Author: aleja
 */

#pragma once

#include "HALAL/Services/PWM/PWM/PWM.hpp"

class DualPWM{
protected:
	DualPWM() = default;
	TimerPeripheral* peripheral;
	uint32_t channel;
	float duty_cycle;
	uint32_t frequency;
	bool is_on = false;
	static constexpr float CLOCK_FREQ_MHZ_WITHOUT_PRESCALER = 275;
	static constexpr float clock_period_ns = (1/CLOCK_FREQ_MHZ_WITHOUT_PRESCALER)*1'000;
	bool is_initialized = false;
public:
	DualPWM(Pin& pin, Pin& pin_negated);

	void turn_on();
	void turn_on_positive();
	void turn_on_negated();
	void turn_off();
	void turn_off_positive();
	void turn_off_negated();
	void set_duty_cycle(float duty_cycle);
	void set_frequency(uint32_t freq_in_hz);
	uint32_t get_frequency()const;
	float get_duty_cycle()const;
	void set_dead_time(std::chrono::nanoseconds dead_time_ns);

};
