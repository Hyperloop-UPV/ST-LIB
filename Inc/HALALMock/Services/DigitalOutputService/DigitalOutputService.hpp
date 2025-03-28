/*
 * DigitalOut.hpp
 *
 *  Created on: Oct 17, 2022
 *      Author: stefan
 */
#pragma once
#include "ErrorHandler/ErrorHandler.hpp"
#include "HALALMock/Models/PinModel/Pin.hpp"
#include "HALALMock/Services/SharedMemory/SharedMemory.hpp"

class DigitalOutputService {
   public:
    static map<uint8_t, Pin> service_ids;
    static uint8_t id_counter;

    static uint8_t inscribe(Pin& pin);
    static void turn_on(uint8_t id);
    static void turn_off(uint8_t id);
    static void set_pin_state(uint8_t id, PinState state);
    static void toggle(uint8_t id);
    static void lock_pin_state(uint8_t id, PinState state);
};
