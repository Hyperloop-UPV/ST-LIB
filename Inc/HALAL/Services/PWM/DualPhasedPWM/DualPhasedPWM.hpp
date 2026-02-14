/*
 * DualPhasedPWMInstance.hpp
 *
 *  Created on: Feb 27, 2023
 *      Author: aleja
 */

#pragma once

#include "HALAL/Services/PWM/PhasedPWM/PhasedPWM.hpp"
#include "HALAL/Services/PWM/DualPWM/DualPWM.hpp"

class DualPhasedPWM : public DualPWM {
private:
    float raw_phase{};

public:
    DualPhasedPWM() = default;
    DualPhasedPWM(Pin& pin, Pin& pin_negated);
    void set_duty_cycle(float duty_cycle);
    void set_frequency(uint32_t frequency);
    void set_raw_phase(float raw_phase);
    void set_phase(float phase_in_deg);
    float get_phase() const;
};
