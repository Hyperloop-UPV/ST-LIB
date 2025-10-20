/*
 * DMA.cpp
 *
 *  Created on: 10 dic. 2022
 *      Author: aleja
 */

#include "HALAL/Models/DMA/DMA.hpp"
#include "ErrorHandler/ErrorHandler.hpp"



void DMA::start() {
	  __HAL_RCC_DMA1_CLK_ENABLE();
	  __HAL_RCC_DMA2_CLK_ENABLE();
	for (Stream dma_stream : inscribed_streams) {
		HAL_NVIC_SetPriority( (IRQn_Type)dma_stream, 0, 0);
		HAL_NVIC_EnableIRQ( (IRQn_Type)dma_stream);
	}
}
