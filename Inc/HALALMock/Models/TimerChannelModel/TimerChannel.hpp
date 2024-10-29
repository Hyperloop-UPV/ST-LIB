/*
 * TimerChannel.hpp
 *
 *  Created on: Nov 1, 2022
 *      Author: alejandro 
 */

#pragma once

#ifdef HAL_TIM_MODULE_ENABLED
struct TimerChannel {
	TIM_HandleTypeDef* timer;
	uint32_t channel;

	bool operator== (const TimerChannel &other) const {
		return (timer == other.timer && channel == other.channel);
	}
	
	bool operator< (const TimerChannel &other) const {
		if (timer == other.timer) { return channel < other.channel; }
		else { return timer < other.timer; }
	}
};
#endif
