/*
 * DigitalOutput.cpp
 *
 *  Created on: 1 dic. 2022
 *      Author: aleja
 */

#include "DigitalOutput/DigitalOutput.hpp"

DigitalOutput::DigitalOutput(Pin& pin)
    : pin(pin), id(DigitalOutputService::inscribe(pin)) {}

void DigitalOutput::turn_on() { DigitalOutputService::turn_on(id); }

void DigitalOutput::turn_off() { DigitalOutputService::turn_off(id); }

void DigitalOutput::set_pin_state(PinState state) {
    DigitalOutputService::set_pin_state(id, state);
}

void DigitalOutput::toggle() { DigitalOutputService::toggle(id); }

bool DigitalOutput::force_lock_pin_state(PinState state) {
    return DigitalOutputService::force_lock_pin_state(id, state);
}