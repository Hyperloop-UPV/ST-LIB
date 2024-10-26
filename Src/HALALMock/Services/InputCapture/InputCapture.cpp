/*
 * InputCapture.cpp
 *
 *  Created on: 30 oct. 2022
 *      Author: alejandro
 */
#include "HALALMock/Services/InputCapture/InputCapture.hpp"
#include "ErrorHandler/ErrorHandler.hpp"

uint8_t InputCapture::id_counter = 0;
map<uint8_t, InputCapture::Instance> InputCapture::active_instances = {};
static map<uint32_t, uint32_t> channel_dict = {
	{HAL_TIM_ACTIVE_CHANNEL_1, TIM_CHANNEL_1},
	{HAL_TIM_ACTIVE_CHANNEL_2, TIM_CHANNEL_2},
	{HAL_TIM_ACTIVE_CHANNEL_3, TIM_CHANNEL_3},
	{HAL_TIM_ACTIVE_CHANNEL_4, TIM_CHANNEL_4},
	{HAL_TIM_ACTIVE_CHANNEL_5, TIM_CHANNEL_5},
	{HAL_TIM_ACTIVE_CHANNEL_6, TIM_CHANNEL_6}
};

InputCapture::Instance::Instance(Pin& pin, TimerPeripheral* peripheral, uint32_t channel_rising, uint32_t channel_falling) :
	pin(pin),
	peripheral(peripheral),
	channel_rising(channel_rising),
	channel_falling(channel_falling)
	{
		frequency = 0;
		duty_cycle = 0;

	}

uint8_t InputCapture::inscribe(Pin& pin){
 	if (not available_instances.contains(pin)) {
		ErrorHandler(" The pin %s is already used or isn t available for InputCapture usage", pin.to_string().c_str());
 		return 0;
 	}

	Pin::inscribe(pin, TIMER_ALTERNATE_FUNCTION);

 	Instance data = available_instances[pin];
	active_instances[id_counter] = data;
	active_instances[id_counter].id = id_counter;

	vector<pair<uint32_t, uint32_t>>& channels = active_instances[id_counter].peripheral->init_data.input_capture_channels;
	uint32_t channel_rising = active_instances[id_counter].channel_rising;
	uint32_t channel_falling = active_instances[id_counter].channel_falling;
	channels.push_back({channel_rising, channel_falling});
	return id_counter++;
}

void InputCapture::turn_on(uint8_t id){
	if (not active_instances.contains(id)) {
		ErrorHandler("ID %d is not registered as an active_instance", id);
		return;
	}
	Instance instance = active_instances[id];


	if (HAL_TIM_IC_Start_IT(instance.peripheral->handle, instance.channel_rising) != HAL_OK) {
		ErrorHandler("Unable to start the %s Input Capture measurement in interrupt mode", instance.peripheral->name.c_str());
	}

	if (HAL_TIM_IC_Start(instance.peripheral->handle, instance.channel_falling) != HAL_OK) {
		ErrorHandler("Unable to start the %s Input Capture measurement", instance.peripheral->name.c_str());
	}

}

void InputCapture::turn_off(uint8_t id){
	if (not active_instances.contains(id)) {
		ErrorHandler("ID %d is not registered as an active_instance", id);
		return;
	}
	Instance instance = active_instances[id];
	if (HAL_TIM_IC_Stop_IT(instance.peripheral->handle, instance.channel_rising) != HAL_OK) {
		ErrorHandler("Unable to stop the %s Input Capture measurement in interrupt mode", instance.peripheral->name.c_str());
	}

	if (HAL_TIM_IC_Stop(instance.peripheral->handle, instance.channel_falling) != HAL_OK) {
		ErrorHandler("Unable to stop the %s Input Capture measurement", instance.peripheral->name.c_str());
	}

}

uint32_t InputCapture::read_frequency(uint8_t id) {
	if (not active_instances.contains(id)) {
		ErrorHandler("ID %d is not registered as an active_instance", id);
		return 0;
	}
	Instance instance = active_instances[id];
	return instance.frequency;
}

uint8_t InputCapture::read_duty_cycle(uint8_t id) {
	if (not active_instances.contains(id)) {
		ErrorHandler("ID %d is not registered as an active_instance", id);
		return 0;
	}
	Instance instance = active_instances[id];
	return instance.duty_cycle;
}

InputCapture::Instance InputCapture::find_instance_by_channel(uint32_t channel) {
	for (auto id_instance: active_instances) {
		uint32_t& ch_rising = id_instance.second.channel_rising;
		uint32_t& ch_falling = id_instance.second.channel_falling;

		if(ch_rising == channel || ch_falling == channel) {
			return id_instance.second;
		}
	}

	ErrorHandler("Channel %d is not a registered channel", channel);
	return Instance();
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	htim->Instance->CNT = 0;
	uint32_t& active_channel = channel_dict[htim->Channel];
	InputCapture::Instance instance = InputCapture::find_instance_by_channel(active_channel);

	uint32_t rising_value = HAL_TIM_ReadCapturedValue(htim, instance.channel_rising);
	if (rising_value != 0) {
		float ref_clock = HAL_RCC_GetPCLK1Freq()*2 / (instance.peripheral->handle->Init.Prescaler+1);
		float falling_value = HAL_TIM_ReadCapturedValue(htim, instance.channel_falling);

		InputCapture::active_instances[instance.id].frequency = round(ref_clock / rising_value);
		InputCapture::active_instances[instance.id].duty_cycle = round((falling_value * 100) / rising_value);
	}

}


