/*
 * DMA.hpp
 *
 *  Created on: 10 dic. 2022
 *      Author: aleja
 */

#pragma once

#include "C++Utilities/CppUtils.hpp"
#include "stm32h7xx_hal.h"
#include "main.h"
#include "HALAL/Models/MPUManager/MPUManager.hpp"
#include <cassert>
#include <variant>
#define MAX_STREAMS 16


// We only have 6 peripherals using DMA for now, the new ssd will need to be added later here


class DMA {
public:
	template<typename T>
	struct Peripheral_type_instance {
		DMA_HandleTypeDef* dma_handle;
		DMA_HandleTypeDef* global_handle;
		T* handle;
		Stream stream; // Maybe dejar IRQn type?
		
	};
	//To allow the array to be multiple types, idk first thing that came to mind
	using InstanceList = std::variant<
		Peripheral_type_instance<ADC_HandleTypeDef>,
		Peripheral_type_instance<I2C_HandleTypeDef>,
		Peripheral_type_instance<FMAC_HandleTypeDef>,
		Peripheral_type_instance<SPI_HandleTypeDef>
	>;

	template<typename T>
	void static inline constexpr inscribe_stream(T* handle);

	static void start();

private:
	inline static constinit uint8_t inscribed_index = 0;
	inline static constinit std::array<InstanceList, MAX_STREAMS> inscribed_streams{};

};