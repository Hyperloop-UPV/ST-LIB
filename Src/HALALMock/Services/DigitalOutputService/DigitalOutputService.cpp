/*
 * DigitalOutputService.cpp
 *
 *  Created on: Oct 18, 2022
 *      Author: stefan
 */

#include "HALALMock/Services/DigitalOutputService/DigitalOutputService.hpp"

uint8_t DigitalOutputService::id_counter = 0;
map<uint8_t, Pin> DigitalOutputService::service_ids = {};

uint8_t DigitalOutputService::inscribe(Pin& pin) {
    Pin::inscribe(pin, OUTPUT);
    DigitalOutputService::service_ids[id_counter] = pin;
    return id_counter++;
}

void DigitalOutputService::turn_off(uint8_t id) {
    if (not DigitalOutputService::service_ids.contains(id)) {
        ErrorHandler("ID %d is not registered as a DigitalOutput", id);
        return;
    }

    Pin pin = DigitalOutputService::service_ids[id];
    EmulatedPin& emulated_pin = SharedMemory::get_pin(pin);
    emulated_pin.PinData.digital_output.state = PinState::OFF;
}

void DigitalOutputService::turn_on(uint8_t id) {
    if (not DigitalOutputService::service_ids.contains(id)) {
        ErrorHandler("ID %d is not registered as a DigitalOutput", id);
        return;
    }

    Pin pin = DigitalOutputService::service_ids[id];
    EmulatedPin& emulated_pin = SharedMemory::get_pin(pin);
    emulated_pin.PinData.digital_output.state = PinState::ON;
}

void DigitalOutputService::set_pin_state(uint8_t id, PinState state) {
    if (not DigitalOutputService::service_ids.contains(id)) {
        ErrorHandler("ID %d is not registered as a DigitalOutput", id);
        return;
    }
    Pin pin = DigitalOutputService::service_ids[id];
    EmulatedPin& emulated_pin = SharedMemory::get_pin(pin);
    emulated_pin.PinData.digital_output.state = state;
}

void DigitalOutputService::toggle(uint8_t id) {
    if (not DigitalOutputService::service_ids.contains(id)) {
        ErrorHandler("ID %d is not registered as a DigitalOutput", id);
        return;
    }

    Pin pin = DigitalOutputService::service_ids[id];
    EmulatedPin& emulated_pin = SharedMemory::get_pin(pin);
    emulated_pin.PinData.digital_output.state == PinState::ON ?
           emulated_pin.PinData.digital_output.state = PinState::OFF : 
           emulated_pin.PinData.digital_output.state = PinState::ON;
}
