/*
 * DMA.cpp
 *
 *  Created on: 10 dic. 2022
 *      Author: aleja
 */

#include "HALAL/Models/DMA/DMA.hpp"
#include "ErrorHandler/ErrorHandler.hpp"


template<typename T>
constexpr void DMA::inscribe_stream(T* handle, DMA_Stream_TypeDef* stream)
{
    Peripheral_type_instance<T> instance{};
    // Verificar que los tipos pasados sean correctos y que aun hay streams sin usar

    // Atributos comunes

    // Atributos propios

    
}


template<typename T>
constexpr void DMA::inscribe_stream(T* handle) 
	{
        Peripheral_type_instance<T> instance{};
		assert(handle != nullptr);
		assert(inscribed_index < DMA_USED); 

		switch (handle->Instance) 
		{
			case ADC1_BASE:
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

			case ADC2_BASE:
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

			case ADC3_BASE:
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
				instance.stream = DMA1Stream2;
				instance.global_handle= &DMA_Handle;

				inscribed_streams[inscribed_index] = instance;
				inscribed_index++;
				break;

			case I2C2_BASE:
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

			case FMAC_BASE:
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

			case SPI3_BASE:
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
            
            default:
                // Aqui poner que diga que el handle esta mal o algo
                ErrorHandler();
                break;
		}

		return;
	}


void DMA::start() 
{
	__HAL_RCC_DMA1_CLK_ENABLE();
	__HAL_RCC_DMA2_CLK_ENABLE();
	for (auto const &inst : inscribed_streams) {
		std::visit([&](auto const &instance) {
			if (HAL_DMA_Init(instance.dma_handle) != HAL_OK)
			{
				Error_Handler();
			}
			__HAL_LINKDMA(instance.handle, instance.global_handle, instance.dma_handle);
			HAL_NVIC_SetPriority((IRQn_Type)instance.stream, 0, 0);
			HAL_NVIC_EnableIRQ((IRQn_Type)instance.stream);
		}, inst);
	}
}
