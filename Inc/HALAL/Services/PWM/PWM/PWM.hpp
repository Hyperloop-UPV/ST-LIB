/*
 * PWM.hpp
 *
 *  Created on: Feb 27, 2023
 *      Author: aleja
 */

#pragma once

#include "HALAL/Models/PinModel/Pin.hpp"
#include "HALAL/Models/TimerPeripheral/TimerPeripheral.hpp"
#include "ErrorHandler/ErrorHandler.hpp"

#ifdef HAL_TIM_MODULE_ENABLED

class PWM {
protected:
	TimerPeripheral* peripheral;
	uint32_t channel;
	float duty_cycle;
	uint32_t frequency;
	bool is_on = false;
	bool is_center_aligned = false;
	static constexpr float CLOCK_FREQ_MHZ_WITHOUT_PRESCALER = 275;
	static constexpr float clock_period_ns = (1/CLOCK_FREQ_MHZ_WITHOUT_PRESCALER)*1'000;
	bool is_initialized = false;
public:
	PWM() = default;
	PWM(Pin& pin);

	void turn_on();
	void turn_off();
	void set_duty_cycle(float duty_cycle);
	void set_frequency(uint32_t frequency);
	uint32_t get_frequency();
	float get_duty_cycle();

	/**
	 * @brief function that sets a deadtime, in which the PWM wouldn t be on HIGH no matter the duty cycle
	 *
	 * 	This function has to be called while the PWM is turned off.
	 * 	This function actually substracts from the HIGH state of the PWM the amount of ns, pulling it down;
	 * 	thus effectively reducing the duty cycle by an amount dependant on the frequency and the dead time.
	 */
	void set_dead_time(std::chrono::nanoseconds dead_time_ns);
};

#endif
