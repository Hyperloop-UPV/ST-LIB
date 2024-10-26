/*
 * DigitalOutputService.cpp
 *
 *  Created on: Oct 18, 2022
 *      Author: stefan
 */

<<<<<<< HEAD

#include "DigitalOutputService/DigitalOutputService.hpp"
#include "ErrorHandler/ErrorHandler.hpp"


=======
#include "HALALMock/Services/DigitalOutputService/DigitalOutputService.hpp"
>>>>>>> origin/development

uint8_t DigitalOutputService::id_counter = 0;
map<uint8_t, Pin> DigitalOutputService::service_ids = {};

<<<<<<< HEAD
uint8_t DigitalOutputService::inscribe(Pin& pin){
        EmulatedPin &pin_data = SharedMemory::get_pin(pin);
		if(pin_data.type == PinType::NOT_USED){
			pin_data.type = PinType::DigitalOutput;
			(pin_data.PinData.DigitalOutput.state)= PinState::OFF;
		}else{
			ErrorHandler("Pin %d is already in use",pin);
			return -1;
		}
		DigitalOutputService::service_ids[id_counter] = pin;
		return id_counter++;
}

void DigitalOutputService::turn_off(uint8_t id){
	if (not DigitalOutputService::service_ids.contains(id)){
		ErrorHandler("ID %d is not registered as a DigitalOutput",id);
		return;
	}
	Pin pin = DigitalOutputService::service_ids[id];
	EmulatedPin &pin_data = SharedMemory::get_pin(pin);
	if(pin_data.type != PinType::DigitalOutput){
		ErrorHandler("ID %d is not registered as a DigitalOutput",id);
		return;
	}
	(pin_data.PinData.DigitalOutput.state)= PinState::OFF;
=======
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
>>>>>>> origin/development

    Pin pin = DigitalOutputService::service_ids[id];
    (*gpio_memory + pin_offset[pin.gpio_pin]) = PinState::OFF;
}

<<<<<<< HEAD
void DigitalOutputService::turn_on(uint8_t id){
	if (not DigitalOutputService::service_ids.contains(id)){
		ErrorHandler("ID %d is not registered as a DigitalOutput",id);
		return;
	}
	Pin pin = DigitalOutputService::service_ids[id];
	EmulatedPin &pin_data = SharedMemory::get_pin(pin);
	if(pin_data.type != PinType::DigitalOutput){
		ErrorHandler("ID %d is not registered as a DigitalOutput",id);
		return;
	}
	(pin_data.PinData.DigitalOutput.state)= PinState::ON;

}

void DigitalOutputService::set_pin_state(uint8_t id, PinState state){
	if (not DigitalOutputService::service_ids.contains(id)){
		ErrorHandler("ID %d is not registered as a DigitalOutput",id);
		return;
	}
	Pin pin = DigitalOutputService::service_ids[id];
	EmulatedPin &pin_data = SharedMemory::get_pin(pin);
	if(pin_data.type != PinType::DigitalOutput){
		ErrorHandler("ID %d is not registered as a DigitalOutput",id);
		return;
	}
	(pin_data.PinData.DigitalOutput.state)= state;

}

void DigitalOutputService::toggle(uint8_t id){
	if (not DigitalOutputService::service_ids.contains(id)){
		ErrorHandler("ID %d is not registered as a DigitalOutput",id);
		return;
	}
	Pin pin = DigitalOutputService::service_ids[id];
	EmulatedPin &pin_data = SharedMemory::get_pin(pin);
	pif(pin_data.type != PinType::DigitalOutput){
		ErrorHandler("ID %d is not registered as a DigitalOutput",id);
		return;
	}
	(pin_data.PinData.DigitalOutput.state)= (pin_data.PinData.DigitalOutput.state) == PinState::ON ? PinState::OFF : PinState::ON;
=======
void DigitalOutputService::turn_on(uint8_t id) {
    if (not DigitalOutputService::service_ids.contains(id)) {
        ErrorHandler("ID %d is not registered as a DigitalOutput", id);
        return;
    }

    Pin pin = DigitalOutputService::service_ids[id];
    (*gpio_memory + pin_offset[pin.gpio_pin]) = PinState::ON;
}

void DigitalOutputService::set_pin_state(uint8_t id, PinState state) {
    if (not DigitalOutputService::service_ids.contains(id)) {
        ErrorHandler("ID %d is not registered as a DigitalOutput", id);
        return;
    }
    Pin pin = DigitalOutputService::service_ids[id];
    (*gpio_memory + pin_offset[pin.gpio_pin]) = state;
}

void DigitalOutputService::toggle(uint8_t id) {
    if (not DigitalOutputService::service_ids.contains(id)) {
        ErrorHandler("ID %d is not registered as a DigitalOutput", id);
        return;
    }

    Pin pin = DigitalOutputService::service_ids[id];
    (*gpio_memory + pin_offset[pin.gpio_pin]) =
        (*gpio_memory + pin_offset[pin.gpio_pin]) == PinState::ON
            ? PinState::OFF
            : PinState::ON;
>>>>>>> origin/development
}
