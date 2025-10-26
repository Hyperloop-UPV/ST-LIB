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
#define STREAMS_USED 10

class DMA {
public:
	template<typename T>
	struct Peripheral_type_instance {
        T* handle;
		DMA_HandleTypeDef* dma_handle;
		DMA_HandleTypeDef* global_handle;
		IRQn_Type irqn; 
	};

	//To allow the array to be multiple types, idk first thing that came to mind
	using InstanceList = std::variant<
		Peripheral_type_instance<ADC_HandleTypeDef>,
		Peripheral_type_instance<I2C_HandleTypeDef>,
		Peripheral_type_instance<FMAC_HandleTypeDef>,
		Peripheral_type_instance<SPI_HandleTypeDef>
	>;

    static constexpr void inscribe_stream_adc(ADC_HandleTypeDef* handle, DMA_Stream_TypeDef* stream);

    static constexpr void inscribe_stream_i2c(I2C_HandleTypeDef* handle, DMA_Stream_TypeDef* stream_rx, DMA_Stream_TypeDef* stream_tx);

    static constexpr void inscribe_stream_spi(SPI_HandleTypeDef* handle, DMA_Stream_TypeDef* stream_rx, DMA_Stream_TypeDef* stream_tx);

    static constexpr void inscribe_stream_fmac(FMAC_HandleTypeDef* handle, DMA_Stream_TypeDef* stream_preload, DMA_Stream_TypeDef* stream_read, DMA_Stream_TypeDef* stream_write);


    template<typename T, typename... Streams>
    constexpr void inscribe_stream(T* handle, Streams*... streams);


    static constexpr uint32_t get_dma_request(const void* instance, const bool mode);

    static constexpr IRQn_Type get_irqn(const DMA_Stream_TypeDef* stream);

	static void start();

private:
	inline static constinit uint8_t inscribed_index = 0;
	inline static constinit std::array<InstanceList, MAX_STREAMS> inscribed_streams{};
    inline static DMA_HandleTypeDef dma_handles[MAX_STREAMS];
};