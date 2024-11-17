#pragma once

#include "HALALMock/Services/PWM/PWM/PWM.hpp"

class DualPWM{
protected:
	DualPWM() = default;
	float* duty_cycle{};
	uint32_t* frequency{};
	bool* positive_is_on{};
	bool* negative_is_on{};
	std::chrono::nanoseconds *dead_time_ns{};
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
