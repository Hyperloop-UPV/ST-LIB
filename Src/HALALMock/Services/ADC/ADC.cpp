/*
 * ADC.cpp
 *
 *  Created on: 20 oct. 2022
 *      Author: alejandro
 */

#include "HALALMock/Services/ADC/ADC.hpp"
#include "ErrorHandler/ErrorHandler.hpp"



uint8_t ADC::id_counter = 0;
unordered_map<uint8_t, ADC::Instance> ADC::active_instances = {};

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
	active_instances[id_counter] = available_instances[pin];
	return id_counter++;
}

void ADC::start() {
	// Storing emulated pins with their corresponding instance
	for (auto& [pin, instance] : available_instances) {
		EmulatedPin& emulated_pin = SharedMemory::get_pin(pin);
		active_emulated_instances[instance] = emulated_pin;
	}
}

void ADC::turn_on(uint8_t id){
	if (not active_instances.contains(id)) {
		return;
	}

	active_instances[id].is_on = true;
}

float ADC::get_value(uint8_t id) {
	Instance& instance = active_instances[id];

	EmulatedPin& emulated_pin = active_emulated_instances[id];
	if (emulated_pin.type != PinType::ADC) {
		ErrorHandler("Pin %s is not configured to be used for ADC usage", emulated_pin);
		return 0;
	}
	ADCResolution resolution = static_cast<ADCResolution>(instance.resolution);
	uint16_t raw = emulated_pin.PinData.ADC.value;
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
	
	EmulatedPin& emulated_pin = active_emulated_instances[id];
	if (emulated_pin.type != PinType::ADC) {
		ErrorHandler("Pin %s is not configured to be used for ADC usage", emulated_pin);
		return 0;
	}

	ADCResolution resolution = static_cast<ADCResolution>(instance.resolution);
	uint16_t raw = emulated_pin.PinData.ADC.value;
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
	return &active_emulated_instances[id].PinData.ADC.value;
}
