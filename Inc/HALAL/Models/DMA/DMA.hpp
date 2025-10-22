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
#define MAX_NUM_STREAMS 15
#define DMA_USED 10		//Temporal name, i cant think of a better one


// We only have 6 peripherals using DMA for now, the new ssd will need to be added later here
extern DMA_HandleTypeDef hdma_adc1;
extern DMA_HandleTypeDef hdma_adc2;
extern DMA_HandleTypeDef hdma_adc3;

extern DMA_HandleTypeDef hdma_i2c2_rx;
extern DMA_HandleTypeDef hdma_i2c2_tx;

extern DMA_HandleTypeDef hdma_fmac_preload;
extern DMA_HandleTypeDef hdma_fmac_read;
extern DMA_HandleTypeDef hdma_fmac_write;

extern DMA_HandleTypeDef hdma_spi3_rx;
extern DMA_HandleTypeDef hdma_spi3_tx;

class DMA {
public:
	enum class Stream : uint8_t {
		DMA1Stream0 = 11,
		DMA1Stream1 = 12,
		DMA1Stream2 = 13,
		DMA1Stream3 = 14,
		DMA1Stream4 = 15,
		DMA1Stream5 = 16,
		DMA1Stream6 = 17,
		DMA2Stream0 = 56,
		DMA2Stream1 = 57,
		DMA2Stream2 = 58,
		DMA2Stream3 = 59,
		DMA2Stream4 = 60,
		DMA2Stream5 = 68,
		DMA2Stream6 = 69,
		DMA2Stream7 = 70,
	};
	
	enum class Peripheral_type {
		Adc1,
		Adc2,
		Adc3,
		I2c2,
		Fmac,
		Spi3
	};
	template<typename T>
	struct Peripheral_type_instance {
		DMA_HandleTypeDef* dma_handle;
		DMA_HandleTypeDef* global_handle;
		T* handle;
		Stream stream;
		
	};
	//To allow the array to be multiple types, idk first thing that came to mind
	using InstanceList = std::variant<
		Peripheral_type_instance<ADC_HandleTypeDef>,
		Peripheral_type_instance<I2C_HandleTypeDef>,
		Peripheral_type_instance<FMAC_HandleTypeDef>,
		Peripheral_type_instance<SPI_HandleTypeDef>
	>;

	template<typename T>
	void static inline constexpr  inscribe_stream(T* handle,Peripheral_type peripheral_type) 
	{
		static_assert(is_variant_member<Instance<T>, MyInstanceVariant>::value,
		"Handle Type not allowed on DMA::inscribe_stream");
		assert(handle != nullptr);
		assert(inscribed_index < DMA_USED); 

		
		switch (peripheral_type) 
		{
			case Peripheral_type::Adc1:
			    static_assert(handle->Instance == ADC1, "Handle passed to DMA::inscribe_stream does not match Peripheral_type::Adc1");
			    Instance<T> instance;
				instance.handle = handle;
				hdma_adc1.Instance = DMA1_Stream0;
				hdma_adc1.Init.Request = DMA_REQUEST_ADC1;
				hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
				hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
				hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
				hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
				hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
				hdma_adc1.Init.Mode = DMA_CIRCULAR;
				hdma_adc1.Init.Priority = DMA_PRIORITY_LOW;
				hdma_adc1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
				instance.dma_handle = &hdma_adc1;
				instance.stream = Stream::DMA1Stream0;
				instance.global_handle= &DMA_Handle

				inscribed_streams[inscribed_index] = instance;
				inscribed_index++;
				break;

			case Peripheral_type::Adc2:
				static_assert(handle->Instance == ADC2, "Handle passed to DMA::inscribe_stream does not match Peripheral_type::Adc2");
				Instance<T> instance;
				instance.handle = handle;
				hdma_adc2.Instance = DMA1_Stream1;
				hdma_adc2.Init.Request = DMA_REQUEST_ADC2;
				hdma_adc2.Init.Direction = DMA_PERIPH_TO_MEMORY;
				hdma_adc2.Init.PeriphInc = DMA_PINC_DISABLE;
				hdma_adc2.Init.MemInc = DMA_MINC_ENABLE;
				hdma_adc2.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
				hdma_adc2.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
				hdma_adc2.Init.Mode = DMA_CIRCULAR;
				hdma_adc2.Init.Priority = DMA_PRIORITY_LOW;
				hdma_adc2.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
				instance.dma_handle = &hdma_adc2;
				instance.stream = Stream::DMA1Stream1;
				instance.global_handle= &DMA_Handle

				inscribed_streams[inscribed_index] = instance;
				inscribed_index++;
				break;

			case Peripheral_type::Adc3:
				static_assert(handle->Instance == ADC3, "Handle passed to DMA::inscribe_stream does not match Peripheral_type::Adc3");
				Instance<T> instance;
				instance.handle = handle;
				hdma_adc3.Instance = DMA1_Stream2;
				hdma_adc3.Init.Request = DMA_REQUEST_ADC3;
				hdma_adc3.Init.Direction = DMA_PERIPH_TO_MEMORY;
				hdma_adc3.Init.PeriphInc = DMA_PINC_DISABLE;
				hdma_adc3.Init.MemInc = DMA_MINC_ENABLE;
				hdma_adc3.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
				hdma_adc3.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
				hdma_adc3.Init.Mode = DMA_CIRCULAR;
				hdma_adc3.Init.Priority = DMA_PRIORITY_LOW;
				hdma_adc3.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
				instance.dma_handle = &hdma_adc3;
				instance.stream = Stream::DMA1Stream2;
				instance.global_handle= &DMA_Handle

				inscribed_streams[inscribed_index] = instance;
				inscribed_index++;
				break;

			case Peripheral_type::I2c2:
				static_assert(handle->Instance == I2C2, "Handle passed to DMA::inscribe_stream does not match Peripheral_type::I2c2");
				Instance<T> instance;
				instance.handle = handle;
				hdma_i2c2_rx.Instance = DMA1_Stream3;
				hdma_i2c2_rx.Init.Request = DMA_REQUEST_I2C2_RX;
				hdma_i2c2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
				hdma_i2c2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
				hdma_i2c2_rx.Init.MemInc = DMA_MINC_ENABLE;
				hdma_i2c2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
				hdma_i2c2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
				hdma_i2c2_rx.Init.Mode = DMA_CIRCULAR;
				hdma_i2c2_rx.Init.Priority = DMA_PRIORITY_LOW;
				hdma_i2c2_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
				instance.dma_handle = &hdma_i2c2_rx; 
				instance.stream = Stream::DMA1Stream3;
				instance.global_handle= &hdmarx;

				inscribed_streams[inscribed_index] = instance;
				inscribed_index++;

				hdma_i2c2_tx.Instance = DMA1_Stream4;
				hdma_i2c2_tx.Init.Request = DMA_REQUEST_I2C2_TX;
				hdma_i2c2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
				hdma_i2c2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
				hdma_i2c2_tx.Init.MemInc = DMA_MINC_ENABLE;
				hdma_i2c2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
				hdma_i2c2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
				hdma_i2c2_tx.Init.Mode = DMA_CIRCULAR;
				hdma_i2c2_tx.Init.Priority = DMA_PRIORITY_LOW;
				hdma_i2c2_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
				instance.dma_handle = &hdma_i2c2_tx;
				instance.stream = Stream::DMA1Stream4;
				instance.global_handle= &hdmatx;


				inscribed_streams[inscribed_index] = instance;
				inscribed_index++;
				break;

			case Peripheral_type::Fmac:
				static_assert(handle->Instance == FMAC, "Handle passed to DMA::inscribe_stream does not match Peripheral_type::Fmac");
				Instance<T> instance;
				hdma_fmac_preload.Instance = DMA2_Stream0;
				hdma_fmac_preload.Init.Request = DMA_REQUEST_MEM2MEM;
				hdma_fmac_preload.Init.Direction = DMA_MEMORY_TO_MEMORY;
				hdma_fmac_preload.Init.PeriphInc = DMA_PINC_ENABLE;
				hdma_fmac_preload.Init.MemInc = DMA_MINC_DISABLE;
				hdma_fmac_preload.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
				hdma_fmac_preload.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
				hdma_fmac_preload.Init.Mode = DMA_NORMAL;
				hdma_fmac_preload.Init.Priority = DMA_PRIORITY_HIGH;
				instance.dma_handle = &hdma_fmac_preload; 
				instance.stream = Stream::DMA2Stream0;
				instance.global_handle= &hdmaPreload;


				inscribed_streams[inscribed_index] = instance;
				inscribed_index++;

				hdma_fmac_write.Instance = DMA2_Stream1;
				hdma_fmac_write.Init.Request = DMA_REQUEST_FMAC_WRITE;
				hdma_fmac_write.Init.Direction = DMA_MEMORY_TO_PERIPH;
				hdma_fmac_write.Init.PeriphInc = DMA_PINC_DISABLE;
				hdma_fmac_write.Init.MemInc = DMA_MINC_ENABLE;
				hdma_fmac_write.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
				hdma_fmac_write.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
				hdma_fmac_write.Init.Mode = DMA_NORMAL;
				hdma_fmac_write.Init.Priority = DMA_PRIORITY_HIGH;
				instance.dma_handle = &hdma_fmac_write;
				instance.stream = Stream::DMA2Stream1;
				instance.global_handle= &hdmaIn;


				inscribed_streams[inscribed_index] = instance;
				inscribed_index++;

				hdma_fmac_read.Instance = DMA2_Stream2;
				hdma_fmac_read.Init.Request = DMA_REQUEST_FMAC_READ;
				hdma_fmac_read.Init.Direction = DMA_PERIPH_TO_MEMORY;
				hdma_fmac_read.Init.PeriphInc = DMA_PINC_DISABLE;
				hdma_fmac_read.Init.MemInc = DMA_MINC_ENABLE;
				hdma_fmac_read.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
				hdma_fmac_read.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
				hdma_fmac_read.Init.Mode = DMA_NORMAL;
				hdma_fmac_read.Init.Priority = DMA_PRIORITY_HIGH;
				instance.dma_handle = &hdma_fmac_read;
				instance.stream = Stream::DMA2Stream2;
				instance.global_handle= &hdmaOut;

				inscribed_streams[inscribed_index] = instance;
				inscribed_index++;
				break;

			case Peripheral_type::Spi3:
				static_assert(handle->Instance == SPI3, "Handle passed to DMA::inscribe_stream does not match Peripheral_type::Spi3");
				Instance<T> instance;
				hdma_spi3_rx.Instance = DMA1_Stream5;
				hdma_spi3_rx.Init.Request = DMA_REQUEST_SPI3_RX;
				hdma_spi3_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
				hdma_spi3_rx.Init.PeriphInc = DMA_PINC_DISABLE;
				hdma_spi3_rx.Init.MemInc = DMA_MINC_ENABLE;
				hdma_spi3_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
				hdma_spi3_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
				hdma_spi3_rx.Init.Mode = DMA_NORMAL;
				hdma_spi3_rx.Init.Priority = DMA_PRIORITY_LOW;
				hdma_spi3_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
				hdma_spi3_rx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
				instance.dma_handle = &hdma_spi3_rx;
				instance.stream = Stream::DMA1Stream5;
				instance.global_handle= &hdmarx;

				inscribed_streams[inscribed_index] = instance;
				inscribed_index++;

				hdma_spi3_tx.Instance = DMA1_Stream6;
				hdma_spi3_tx.Init.Request = DMA_REQUEST_SPI3_TX;
				hdma_spi3_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
				hdma_spi3_tx.Init.PeriphInc = DMA_PINC_DISABLE;
				hdma_spi3_tx.Init.MemInc = DMA_MINC_ENABLE;
				hdma_spi3_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
				hdma_spi3_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
				hdma_spi3_tx.Init.Mode = DMA_NORMAL;
				hdma_spi3_tx.Init.Priority = DMA_PRIORITY_LOW;
				hdma_spi3_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
				hdma_spi3_tx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
				instance.dma_handle = &hdma_spi3_tx;
				instance.stream = Stream::DMA1Stream6;
				instance.global_handle= &hdmatx;

				inscribed_streams[inscribed_index] = instance;
				inscribed_index++;
				break;
		}

		return;
	}

	static void start();

private:
	inline static constinit uint8_t inscribed_index = 0;
	inline static constinit std::array<InstanceList, DMA_USED> inscribed_streams{};
	static constexpr std::array<Stream, 15> streams = {
    Stream::DMA1Stream0, Stream::DMA1Stream1, Stream::DMA1Stream2, Stream::DMA1Stream3,
    Stream::DMA1Stream4, Stream::DMA1Stream5, Stream::DMA1Stream6,
    Stream::DMA2Stream0, Stream::DMA2Stream1, Stream::DMA2Stream2, Stream::DMA2Stream3,
    Stream::DMA2Stream4, Stream::DMA2Stream5, Stream::DMA2Stream6, Stream::DMA2Stream7
	};
};