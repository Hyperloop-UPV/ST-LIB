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
			__HAL_LINKDMA(instance->handle, instance.global_handle, instance->dma_handle);
			HAL_NVIC_SetPriority(instance.irqn, 0, 0);
			HAL_NVIC_EnableIRQ(instance.irqn);
		}, inst);
	}
}


template<typename T, typename... Streams>
constexpr void DMA::inscribe_stream(T* handle, Streams*... streams){
    if constexpr (sizeof...(Streams) > MAX_STREAMS){
        ErrorHandler("The maximum number of streams is %d", MAX_STREAMS);
    }
    uint8_t num_streams = sizeof...(Streams);
    
    // Verify that the number of streams is correct for different peripherials
    if constexpr (std::is_same<T, ADC_HandleTypeDef> && num_streams != 1){
        ErrorHandler("For ADC, there must be only one stream");
    }
    else if constexpr (std::is_same<T, SPI_HandleTypeDef> && num_streams != 2){
        ErrorHandler("For SPI, there must be two streams (RX and TX)");
    }
    else if constexpr (std::is_same<T, I2C_HandleTypeDef> && num_streams != 2){
        ErrorHandler("For I2C, there must be two streams (RX and TX)");
    }
    else if constexpr (std::is_same<T, FMAC_HandleTypeDef> && num_streams != 3){
        ErrorHandler("For FMAC, there must be three streams (preload, write and read)");
    }


    Peripheral_type_instance<T> instances[num_streams];
    
    // All of the instances of the same peripherial will have the same handle
    for constexpr (uint8_t num : num_streams){
        instances[num].handle = handle;
    }

}
