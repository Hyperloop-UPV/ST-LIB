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

void DigitalOutput::toggle() {
    BENCHMARK_BEGIN(SIMPLE_MARK,2,"DigitalOutputService::toggle()")
    DigitalOutputService::toggle(id); 
    BENCHMARK_END(SIMPLE_MARK,2)
}

bool DigitalOutput::lock_pin_state(PinState state) {
    return DigitalOutputService::lock_pin_state(id, state);
}