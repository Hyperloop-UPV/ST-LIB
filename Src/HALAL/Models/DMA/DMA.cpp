/*
 * DMA.cpp
 *
 *  Created on: 10 dic. 2022
 *      Author: aleja
 */

#include "HALAL/Models/DMA/DMA.hpp"
#include "ErrorHandler/ErrorHandler.hpp"


constexpr uint32_t DMA::get_dma_request(const void* instance, const bool mode) {
    if (instance == ADC1) return DMA_REQUEST_ADC1;
    if (instance == ADC2) return DMA_REQUEST_ADC2;
    if (instance == ADC3) return DMA_REQUEST_ADC3;

    if (instance == I2C1 && mode) return DMA_REQUEST_I2C1_RX;
    if (instance == I2C1 && !mode) return DMA_REQUEST_I2C1_TX;
    if (instance == I2C2 && mode) return DMA_REQUEST_I2C2_RX;
    if (instance == I2C2 && !mode) return DMA_REQUEST_I2C2_TX;
    if (instance == I2C3 && mode) return DMA_REQUEST_I2C3_RX;
    if (instance == I2C3 && !mode) return DMA_REQUEST_I2C3_TX;
    if (instance == I2C5 && mode) return DMA_REQUEST_I2C5_RX; // NO hay 4?
    if (instance == I2C5 && !mode) return DMA_REQUEST_I2C5_TX;

    if (instance == SPI1 && mode) return DMA_REQUEST_SPI1_RX;
    if (instance == SPI1 && !mode) return DMA_REQUEST_SPI1_TX;
    if (instance == SPI2 && mode) return DMA_REQUEST_SPI2_RX;
    if (instance == SPI2 && !mode) return DMA_REQUEST_SPI2_TX;
    if (instance == SPI3 && mode) return DMA_REQUEST_SPI3_RX;
    if (instance == SPI3 && !mode) return DMA_REQUEST_SPI3_TX;
    if (instance == SPI4 && mode) return DMA_REQUEST_SPI4_RX;
    if (instance == SPI4 && !mode) return DMA_REQUEST_SPI4_TX;
    if (instance == SPI5 && mode) return DMA_REQUEST_SPI5_RX;
    if (instance == SPI5 && !mode) return DMA_REQUEST_SPI5_TX; // NO hay 6?
    
    return 0;
}


constexpr IRQn_Type DMA::get_irqn(const DMA_Stream_TypeDef* stream) {
    if (stream == DMA1_Stream0) return DMA1_Stream0_IRQn;
    if (stream == DMA1_Stream1) return DMA1_Stream1_IRQn;
    if (stream == DMA1_Stream2) return DMA1_Stream2_IRQn;
    if (stream == DMA1_Stream3) return DMA1_Stream3_IRQn;
    if (stream == DMA1_Stream4) return DMA1_Stream4_IRQn;
    if (stream == DMA1_Stream5) return DMA1_Stream5_IRQn;
    if (stream == DMA1_Stream6) return DMA1_Stream6_IRQn;
    if (stream == DMA1_Stream7) return DMA1_Stream7_IRQn;

    if (stream == DMA2_Stream0) return DMA2_Stream0_IRQn;
    if (stream == DMA2_Stream1) return DMA2_Stream1_IRQn;
    if (stream == DMA2_Stream2) return DMA2_Stream2_IRQn;
    if (stream == DMA2_Stream3) return DMA2_Stream3_IRQn;
    if (stream == DMA2_Stream4) return DMA2_Stream4_IRQn;
    if (stream == DMA2_Stream5) return DMA2_Stream5_IRQn;
    if (stream == DMA2_Stream6) return DMA2_Stream6_IRQn;
    if (stream == DMA2_Stream7) return DMA2_Stream7_IRQn;
}



constexpr void DMA::inscribe_stream_adc(ADC_HandleTypeDef* handle, DMA_Stream_TypeDef* stream)
{
    Peripheral_type_instance<ADC_HandleTypeDef> instance{};
    static DMA_HandleTypeDef dma_handle;

    instance.handle = handle;
    dma_handle.Instance = stream;
    dma_handle.Init.Request = get_dma_request(handle->Instance, true); // El modo no hace nada aquí
    dma_handle.Init.Direction = DMA_PERIPH_TO_MEMORY;
    dma_handle.Init.PeriphInc = DMA_PINC_DISABLE;
    dma_handle.Init.MemInc = DMA_MINC_ENABLE;
    dma_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    dma_handle.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    dma_handle.Init.Mode = DMA_CIRCULAR;
    dma_handle.Init.Priority = DMA_PRIORITY_LOW;
    dma_handle.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    instance.global_handle = handle->DMA_Handle;
    instance.irqn = get_irqn(stream);
    instance.dma_handle = &dma_handle;
    
    inscribed_streams[inscribed_index] = instance;
    inscribed_index++;
}


constexpr void DMA::inscribe_stream_i2c(I2C_HandleTypeDef* handle, DMA_Stream_TypeDef* stream_rx, DMA_Stream_TypeDef* stream_tx)
{
    // RX Stream
    Peripheral_type_instance<I2C_HandleTypeDef> instance_rx{};
    static DMA_HandleTypeDef dma_handle_rx;
    
    instance_rx.handle = handle;
    dma_handle_rx.Instance = stream_rx;
    dma_handle_rx.Init.Request = get_dma_request(handle->Instance, true); 
    dma_handle_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    dma_handle_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    dma_handle_rx.Init.MemInc = DMA_MINC_ENABLE;
    dma_handle_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    dma_handle_rx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    dma_handle_rx.Init.Mode = DMA_CIRCULAR;
    dma_handle_rx.Init.Priority = DMA_PRIORITY_LOW;
    dma_handle_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    instance_rx.dma_handle = &dma_handle_rx;
    instance_rx.global_handle = handle->hdmarx;
    instance_rx.irqn = get_irqn(stream_rx);

    inscribed_streams[inscribed_index] = instance_rx;
    inscribed_index++;

    // TX Stream
    Peripheral_type_instance<I2C_HandleTypeDef> instance_tx{};
    static DMA_HandleTypeDef dma_handle_tx;
    
    instance_tx.handle = handle;
    dma_handle_tx.Instance = stream_tx;
    dma_handle_rx.Init.Request = get_dma_request(handle->Instance, false); 
    dma_handle_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    dma_handle_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    dma_handle_tx.Init.MemInc = DMA_MINC_ENABLE;
    dma_handle_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    dma_handle_tx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    dma_handle_tx.Init.Mode = DMA_CIRCULAR;
    dma_handle_tx.Init.Priority = DMA_PRIORITY_LOW;
    dma_handle_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    instance_tx.dma_handle = &dma_handle_tx;
    instance_tx.global_handle = handle->hdmatx;
    instance_tx.irqn = get_irqn(stream_tx);

    inscribed_streams[inscribed_index] = instance_tx;
    inscribed_index++;
}


constexpr void DMA::inscribe_stream_spi(SPI_HandleTypeDef* handle, DMA_Stream_TypeDef* stream_rx, DMA_Stream_TypeDef* stream_tx)
{
    // RX Stream
    Peripheral_type_instance<SPI_HandleTypeDef> instance_rx{};
    static DMA_HandleTypeDef dma_handle_rx;
    
    instance_rx.handle = handle;
    dma_handle_rx.Instance = stream_rx;
    dma_handle_rx.Init.Request = get_dma_request(handle->Instance, true);
    dma_handle_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    dma_handle_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    dma_handle_rx.Init.MemInc = DMA_MINC_ENABLE;
    dma_handle_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    dma_handle_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    dma_handle_rx.Init.Mode = DMA_NORMAL;
    dma_handle_rx.Init.Priority = DMA_PRIORITY_LOW;
    dma_handle_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    dma_handle_rx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    instance_rx.dma_handle = &dma_handle_rx;
    instance_rx.global_handle = handle->hdmarx;
    instance_rx.irqn = get_irqn(stream_rx);

    inscribed_streams[inscribed_index] = instance_rx;
    inscribed_index++;

    // TX Stream
    Peripheral_type_instance<SPI_HandleTypeDef> instance_tx{};
    static DMA_HandleTypeDef dma_handle_tx;
    
    instance_tx.handle = handle;
    dma_handle_tx.Instance = stream_tx;
    dma_handle_tx.Init.Request = get_dma_request(handle->Instance, false);
    dma_handle_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    dma_handle_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    dma_handle_tx.Init.MemInc = DMA_MINC_ENABLE;
    dma_handle_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    dma_handle_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    dma_handle_tx.Init.Mode = DMA_NORMAL;
    dma_handle_tx.Init.Priority = DMA_PRIORITY_LOW;
    dma_handle_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    dma_handle_tx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    instance_tx.dma_handle = &dma_handle_tx;
    instance_tx.global_handle = handle->hdmatx;
    instance_tx.irqn = get_irqn(stream_tx);

    inscribed_streams[inscribed_index] = instance_tx;
    inscribed_index++;
}


constexpr void DMA::inscribe_stream_fmac(FMAC_HandleTypeDef* handle, DMA_Stream_TypeDef* stream_preload, DMA_Stream_TypeDef* stream_read, DMA_Stream_TypeDef* stream_write)
{
    Peripheral_type_instance<FMAC_HandleTypeDef> instance_preload{};
    static DMA_HandleTypeDef dma_handle_preload;
    
    instance_preload.handle = handle;
    dma_handle_preload.Instance = stream_preload;
    dma_handle_preload.Init.Request = DMA_REQUEST_MEM2MEM;
    dma_handle_preload.Init.Direction = DMA_MEMORY_TO_MEMORY;
    dma_handle_preload.Init.PeriphInc = DMA_PINC_ENABLE;
    dma_handle_preload.Init.MemInc = DMA_MINC_DISABLE;
    dma_handle_preload.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    dma_handle_preload.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    dma_handle_preload.Init.Mode = DMA_NORMAL;
    dma_handle_preload.Init.Priority = DMA_PRIORITY_HIGH;
    instance_preload.dma_handle = &dma_handle_preload;
    instance_preload.global_handle = handle->hdmaPreload;
    instance_preload.irqn = get_irqn(stream_preload);

    inscribed_streams[inscribed_index] = instance_preload;
    inscribed_index++;

    Peripheral_type_instance<FMAC_HandleTypeDef> instance_write{};
    static DMA_HandleTypeDef dma_handle_write;
    
    instance_write.handle = handle;
    dma_handle_write.Instance = stream_write;
    dma_handle_write.Init.Request = DMA_REQUEST_FMAC_WRITE;
    dma_handle_write.Init.Direction = DMA_MEMORY_TO_PERIPH;
    dma_handle_write.Init.PeriphInc = DMA_PINC_DISABLE;
    dma_handle_write.Init.MemInc = DMA_MINC_ENABLE;
    dma_handle_write.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    dma_handle_write.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    dma_handle_write.Init.Mode = DMA_NORMAL;
    dma_handle_write.Init.Priority = DMA_PRIORITY_HIGH;
    instance_write.dma_handle = &dma_handle_write;
    instance_write.global_handle = handle->hdmaIn;
    instance_write.irqn = get_irqn(stream_write);

    inscribed_streams[inscribed_index] = instance_write;
    inscribed_index++;


    Peripheral_type_instance<FMAC_HandleTypeDef> instance_read{};
    static DMA_HandleTypeDef dma_handle_read;
    
    instance_read.handle = handle;
    dma_handle_read.Instance = stream_read;
    dma_handle_read.Init.Request = DMA_REQUEST_FMAC_READ;
    dma_handle_read.Init.Direction = DMA_PERIPH_TO_MEMORY;
    dma_handle_read.Init.PeriphInc = DMA_PINC_DISABLE;
    dma_handle_read.Init.MemInc = DMA_MINC_ENABLE;
    dma_handle_read.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    dma_handle_read.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    dma_handle_read.Init.Mode = DMA_NORMAL;
    dma_handle_read.Init.Priority = DMA_PRIORITY_HIGH;
    instance_read.dma_handle = &dma_handle_read;
    instance_read.global_handle = handle->hdmaOut;
    instance_read.irqn = get_irqn(stream_read);

    inscribed_streams[inscribed_index] = instance_read;
    inscribed_index++;
}


template<typename T>
constexpr void DMA::inscribe_stream(T* handle) 
	{
        Peripheral_type_instance<T> instance{};
		assert(handle != nullptr);
		assert(inscribed_index < DMA_USED); 
        handle->DMA_Handle
		switch (handle->Instance) 
		{
			case ADC1_BASE:
				
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
			HAL_NVIC_SetPriority(instance.irqn, 0, 0);
			HAL_NVIC_EnableIRQ(instance.irqn);
		}, inst);
	}
}
