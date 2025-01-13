/*
 * ADC.cpp
 *
 *  Created on: 20 oct. 2022
 *      Author: alejandro
 */

#include "HALALMock/Services/ADC/ADC.hpp"
#include "ErrorHandler/ErrorHandler.hpp"



uint8_t ADC::id_counter{0};
unordered_map<uint8_t, ADC::Instance> ADC::active_instances{};
unordered_map<uint8_t, EmulatedPin*> ADC::active_emulated_instances{};
unordered_map<uint8_t, Pin> ADC::id_to_pin{}; 

uint8_t ADC::inscribe(Pin pin) {
	if (not available_instances.contains(pin)) {
		ErrorHandler("Pin %s is already used or isn t available for ADC usage", pin.to_string().c_str());
		return 0;
	}

	// Emulated pin inscribe
	EmulatedPin& emulated_pin = SharedMemory::get_pin(pin);
	if (emulated_pin.type != PinType::NOT_USED) {
		ErrorHandler("Pin %s is not available for ADC usage, is already using as %s", pin.to_string().c_str(), emulated_pin.type);
		return 0;
	}

	Pin::inscribe(pin, OperationMode::ANALOG);
	emulated_pin.type = PinType::ADC;
	active_instances[id_counter] = available_instances[pin];
	id_to_pin[id_counter] = pin;
	return id_counter++;
}

void ADC::start() {
    // Clear any existing mappings
    active_emulated_instances.clear();

    // Map each active ADC ID to its corresponding EmulatedPin
    for (const auto& [id, instance] : active_instances) {
        auto pin_it = id_to_pin.find(id);
        if (pin_it == id_to_pin.end()) {
            ErrorHandler("No pin mapped for ADC ID %u", id);
            continue;
        }
        Pin pin = pin_it->second;
        EmulatedPin& emulated_pin = SharedMemory::get_pin(pin);
        active_emulated_instances[id] = &emulated_pin;
		active_emulated_instances[id]->PinData.adc.resolution=instance.resolution;
    }
}

void ADC::turn_on(uint8_t id){
	if (not active_instances.contains(id)) {
		return;
	}

	active_instances[id].is_on = true;
	active_emulated_instances[id]->PinData.adc.is_on = true;
}

float ADC::get_value(uint8_t id) {
	Instance& instance = active_instances[id];

	EmulatedPin* emulated_pin = active_emulated_instances[id];
	if (emulated_pin->type != PinType::ADC) {
		ErrorHandler("Pin %s is not configured to be used for ADC usage", emulated_pin);
		return 0;
	}
	ADCResolution resolution = static_cast<ADCResolution>(instance.resolution);
	uint16_t raw = emulated_pin->PinData.adc.value;
	switch (resolution) {
		case ADCResolution::ADC_RES_16BITS:
			return (raw * ADC_MAX_VOLTAGE) / (float)MAX_16BIT;
		case ADCResolution::ADC_RES_14BITS:
			return (raw * ADC_MAX_VOLTAGE) / (float)MAX_14BIT;
		case ADCResolution::ADC_RES_12BITS:
			return (raw * ADC_MAX_VOLTAGE) / (float)MAX_12BIT;
		case ADCResolution::ADC_RES_10BITS:
			return (raw * ADC_MAX_VOLTAGE) / (float)MAX_10BIT;
		default:
			ErrorHandler("ADC Resolution not supported");
			return 0;
	}
}

uint16_t ADC::get_int_value(uint8_t id) {
	Instance& instance = active_instances[id];
	
	EmulatedPin* emulated_pin = active_emulated_instances[id];
	if (emulated_pin->type != PinType::ADC) {
		ErrorHandler("Pin %s is not configured to be used for ADC usage", emulated_pin);
		return 0;
	}

	ADCResolution resolution = static_cast<ADCResolution>(instance.resolution);
	uint16_t raw = emulated_pin->PinData.adc.value;
	switch (resolution) {
		case ADCResolution::ADC_RES_16BITS:
			return raw;
		case ADCResolution::ADC_RES_14BITS:
			return raw << 2;
		case ADCResolution::ADC_RES_12BITS:
			return raw << 4;
		case ADCResolution::ADC_RES_10BITS:
			return raw << 6;
		default:
			ErrorHandler("ADC Resolution not supported");
			return 0;
	}
}

uint16_t* ADC::get_value_pointer(uint8_t id) {
	if (active_emulated_instances.find(id) != active_emulated_instances.end())
        return &active_emulated_instances[id]->PinData.adc.value;
    else
		return nullptr;
}
