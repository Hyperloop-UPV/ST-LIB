/*
 * HalfBridge.hpp
 *
 *  Created on: Dec 1, 2022
 *      Author: aleja
 */

#pragma once

#include <cstdint>

#include "HALAL/Models/PinModel/Pin.hpp"
#include "HALAL/Services/PWM/DualPhasedPWM/DualPhasedPWM.hpp"
#include "ErrorHandler/ErrorHandler.hpp"

class HalfBridge {
public:
    HalfBridge() = default;
    HalfBridge(
        Pin& positive_pwm_pin,
        Pin& positive_pwm_negated_pin,
        Pin& negative_pwm_pin,
        Pin& negative_pwm_negated_pin,
        Pin& enable_pin
    );

    void turn_on();
    void turn_off();
    void set_duty_cycle(float duty_cycle);
    void set_frequency(int32_t frequency);
    void set_phase(float phase);
    void set_positive_pwm_phase(float phase);
    void set_negative_pwm_phase(float phase);
    float get_phase();

private:
    bool is_dual;
    DualPhasedPWM positive_pwm;
    DualPhasedPWM negative_pwm;

    uint8_t enable;
};
