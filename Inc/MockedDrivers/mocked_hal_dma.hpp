#pragma once

#include "hal_wrapper.h"

#include <cstddef>

namespace ST_LIB::MockedHAL {

enum class DMAOperation : uint8_t {
  Init = 0,
  StartIT,
  IRQHandler,
};

void dma_reset();
void dma_set_init_status(HAL_StatusTypeDef status);
void dma_set_start_status(HAL_StatusTypeDef status);

std::size_t dma_get_call_count(DMAOperation op);
DMA_HandleTypeDef *dma_get_last_init_handle();
DMA_HandleTypeDef *dma_get_last_start_handle();
DMA_HandleTypeDef *dma_get_last_irq_handle();
uint32_t dma_get_last_start_src();
uint32_t dma_get_last_start_dst();
uint32_t dma_get_last_start_length();

} // namespace ST_LIB::MockedHAL

