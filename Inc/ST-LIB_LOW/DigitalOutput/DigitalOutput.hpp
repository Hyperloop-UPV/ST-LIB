/*
 * DigitalOutput.hpp
 *
 *  Created on: 1 dic. 2022
 *      Author: aleja
 */

#pragma once

#include <cstdint>

#include "HALAL/Models/PinModel/Pin.hpp"

class DigitalOutput {
public:
    DigitalOutput() = default;
    DigitalOutput(Pin& pin);

    void turn_on();
    void turn_off();
    void toggle();
    void set_pin_state(PinState state);
    bool lock_pin_state(PinState state);

private:
    Pin pin;
    uint8_t id;
};
