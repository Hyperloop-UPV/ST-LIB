/*
 * DMA.cpp
 *
 *  Created on: 10 dic. 2022
 *      Author: aleja
 */

#include "HALAL/Models/DMA/DMA.hpp"
#include "ErrorHandler/ErrorHandler.hpp"

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
