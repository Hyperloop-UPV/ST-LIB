/*
 * ADC.cpp
 *
 *  Created on: 20 oct. 2022
 *      Author: alejandro
 */

#include "HALALMock/Services/ADC/ADC.hpp"
#include "ErrorHandler/ErrorHandler.hpp"

#if defined(HAL_ADC_MODULE_ENABLED) && defined(HAL_LPTIM_MODULE_ENABLED)

extern ADC_HandleTypeDef hadc3;

uint8_t ADC::id_counter = 0;
unordered_map<uint8_t, ADC::Instance> ADC::active_instances = {};


ADC::InitData::InitData(ADC_TypeDef* adc, uint32_t resolution, uint32_t external_trigger, vector<uint32_t>& channels, DMA::Stream dma_stream, string name) :
		adc(adc), resolution(resolution), external_trigger(external_trigger), channels(channels), dma_stream(dma_stream), name(name) {}

ADC::Peripheral::Peripheral(ADC_HandleTypeDef* handle, LowPowerTimer& timer, InitData& init_data) :
	handle(handle), timer(timer), init_data(init_data) {
}

bool ADC::Peripheral::is_registered() {
	return init_data.channels.size();
}

ADC::Instance::Instance(ADC::Peripheral* peripheral, uint32_t channel) :
		peripheral(peripheral), channel(channel) {}

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

	InitData& init_data = active_instances[id_counter].peripheral->init_data;
	DMA::inscribe_stream(init_data.dma_stream);
	active_instances[id_counter].rank = init_data.channels.size();
	init_data.channels.push_back(active_instances[id_counter].channel);
	return id_counter++;
}

void ADC::start() {
	for(Peripheral& peripheral : peripherals) {
		if (peripheral.is_registered()) {
			ADC::init(peripheral);
		}
	}

	// Storing emulated pins with their corresponding instance
	for (auto& [pin, instance] : available_instances) {
		EmulatedPin& emulated_pin = SharedMemory::get_pin(pin);
		available_emulated_instances[instance] = emulated_pin;
	}
}

void ADC::turn_on(uint8_t id){
	if (not active_instances.contains(id)) {
		return;
	}

	Peripheral* peripheral = active_instances[id].peripheral;
	if (peripheral->is_on) {
		return;
	}

	peripheral->is_on = true;
}

float ADC::get_value(uint8_t id) {
	Instance& instance = active_instances[id];

	EmulatedPin& emulated_pin = available_emulated_instances[instance];
	if (emulated_pin.type != PinType::ADC) {
		ErrorHandler("Pin %s is not configured to be used for ADC usage", emulated_pin);
		return 0;
	}
	ADCResolution resolution = static_cast<ADCResolution>(instance.peripheral->init_data.resolution);
	uint16_t raw = emulated_pin.PinData.ADC.value;
	switch (resolution) {
		case ADCResolution::ADC_RES_16BITS:
			return raw / MAX_16BIT * ADC_MAX_VOLTAGE;
		case ADCResolution::ADC_RES_14BITS:
			return raw / MAX_14BIT * ADC_MAX_VOLTAGE;
		case ADCResolution::ADC_RES_12BITS:
			return raw / MAX_12BIT * ADC_MAX_VOLTAGE;
		case ADCResolution::ADC_RES_10BITS:
			return raw / MAX_10BIT * ADC_MAX_VOLTAGE;
		default:
			ErrorHandler("ADC Resolution not supported");
			return 0;
	}
}

uint16_t ADC::get_int_value(uint8_t id) {
	Instance& instance = active_instances[id];

	EmulatedPin& emulated_pin = available_emulated_instances[instance];
	if (emulated_pin.type != PinType::ADC) {
		ErrorHandler("Pin %s is not configured to be used for ADC usage", emulated_pin);
		return 0;
	}

	uint16_t raw = emulated_pin.PinData.ADC.value;

	ADCResolution resolution = static_cast<ADCResolution>(instance.peripheral->init_data.resolution);
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
	Instance& instance = active_instances[id];
	return &available_emulated_instances[instance].PinData.ADC.value;
}

// Empty function because no need to initialize the ADC in the mock
void ADC::init(Peripheral& peripheral) {}
#endif
