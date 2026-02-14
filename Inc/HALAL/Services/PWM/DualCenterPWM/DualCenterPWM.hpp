#pragma once

#include "HALAL/Services/PWM/DualPWM/DualPWM.hpp"

class DualCenterPWM : public DualPWM {
public:
    DualCenterPWM(Pin& pin, Pin& pin_negated);

    void set_frequency(uint32_t freq_in_hz);
};