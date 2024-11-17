#pragma once
#include "ErrorHandler/ErrorHandler.hpp"
#include "HALALMock/Models/PinModel/Pin.hpp"
#include "HALALMock/Services/SharedMemory/SharedMemory.hpp"

class PWM {
protected:
	float *duty_cycle;
	uint32_t *frequency;
	bool *is_on;
	std::chrono::nanoseconds *dead_time_ns;
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
