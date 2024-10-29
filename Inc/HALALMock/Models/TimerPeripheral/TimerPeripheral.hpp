/*
 * TimerPeripheral.hpp
 *
 *  Created on: 3 dic. 2022
 *      Author: aleja
 */

#pragma once
#include "stm32h7xx_hal.h"

#ifdef HAL_TIM_MODULE_ENABLED

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim8;
extern TIM_HandleTypeDef htim12;
extern TIM_HandleTypeDef htim15;
extern TIM_HandleTypeDef htim16;
extern TIM_HandleTypeDef htim17;
extern TIM_HandleTypeDef htim23;

#include "C++Utilities/CppUtils.hpp"
#include "ErrorHandler/ErrorHandler.hpp"

#define PWMmap map<Pin, pair<reference_wrapper<TimerPeripheral>, TimerPeripheral::PWMData>>
#define DualPWMmap map<pair<Pin, Pin>, pair<reference_wrapper<TimerPeripheral>, TimerPeripheral::PWMData>>

class TimerPeripheral {
public:
	enum PWM_MODE : uint8_t {
		NORMAL = 0,
		PHASED = 1
	};

	enum TIM_TYPE {
		BASE,
		ADVANCED
	};

	struct PWMData {
		uint32_t channel;
		PWM_MODE mode;

	};

	struct InitData {
	private:
		InitData() = default;
	public:
		TIM_TYPE type;
		uint32_t prescaler;
		uint32_t period;
		uint32_t deadtime;
		uint32_t polarity;
		uint32_t negated_polarity;
		vector<PWMData> pwm_channels = {};
		vector<pair<uint32_t, uint32_t>> input_capture_channels = {};
		InitData(TIM_TYPE type, uint32_t prescaler = 5,
				uint32_t period = 55000, uint32_t deadtime = 0, uint32_t polarity = TIM_OCPOLARITY_HIGH, uint32_t negated_polarity = TIM_OCPOLARITY_HIGH);
	};

	TIM_HandleTypeDef* handle;
	InitData init_data;
	string name;

	static PWMmap available_pwm;
	static DualPWMmap available_dual_pwms;
	static vector<reference_wrapper<TimerPeripheral>> timers;

	TimerPeripheral() = default;
	TimerPeripheral(TIM_HandleTypeDef* timer, InitData&& init_data, string name);

	static void start();

	void init();
	bool is_registered();
	uint32_t get_prescaler();
	uint32_t get_period();
	bool is_occupied();

private:
	static map<TIM_HandleTypeDef*, TIM_TypeDef*> handle_to_timer;

	friend class Encoder;
};

#endif
