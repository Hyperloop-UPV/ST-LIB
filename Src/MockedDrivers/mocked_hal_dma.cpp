#include "MockedDrivers/mocked_hal_dma.hpp"

#include <array>

namespace {

struct DMAState {
    HAL_StatusTypeDef init_status = HAL_OK;
    HAL_StatusTypeDef start_status = HAL_OK;
    std::array<std::size_t, 3> calls{};
    DMA_HandleTypeDef* last_init_handle = nullptr;
    DMA_HandleTypeDef* last_start_handle = nullptr;
    DMA_HandleTypeDef* last_irq_handle = nullptr;
    uint32_t last_start_src = 0;
    uint32_t last_start_dst = 0;
    uint32_t last_start_length = 0;
};

DMAState g_state{};

} // namespace

namespace ST_LIB::MockedHAL {

void dma_reset() { g_state = {}; }

void dma_set_init_status(HAL_StatusTypeDef status) { g_state.init_status = status; }

void dma_set_start_status(HAL_StatusTypeDef status) { g_state.start_status = status; }

std::size_t dma_get_call_count(DMAOperation op) {
    return g_state.calls[static_cast<std::size_t>(op)];
}

DMA_HandleTypeDef* dma_get_last_init_handle() { return g_state.last_init_handle; }

DMA_HandleTypeDef* dma_get_last_start_handle() { return g_state.last_start_handle; }

DMA_HandleTypeDef* dma_get_last_irq_handle() { return g_state.last_irq_handle; }

uint32_t dma_get_last_start_src() { return g_state.last_start_src; }

uint32_t dma_get_last_start_dst() { return g_state.last_start_dst; }

uint32_t dma_get_last_start_length() { return g_state.last_start_length; }

} // namespace ST_LIB::MockedHAL

extern "C" HAL_StatusTypeDef MockedHAL_DMA_Init_Impl(DMA_HandleTypeDef* hdma) {
    g_state.calls[static_cast<std::size_t>(ST_LIB::MockedHAL::DMAOperation::Init)]++;
    g_state.last_init_handle = hdma;
    if (hdma == nullptr) {
        return HAL_ERROR;
    }
    return g_state.init_status;
}

extern "C" HAL_StatusTypeDef MockedHAL_DMA_Start_IT_Impl(
    DMA_HandleTypeDef* hdma,
    uint32_t SrcAddress,
    uint32_t DstAddress,
    uint32_t DataLength
) {
    g_state.calls[static_cast<std::size_t>(ST_LIB::MockedHAL::DMAOperation::StartIT)]++;
    g_state.last_start_handle = hdma;
    g_state.last_start_src = SrcAddress;
    g_state.last_start_dst = DstAddress;
    g_state.last_start_length = DataLength;
    if (hdma == nullptr) {
        return HAL_ERROR;
    }
    return g_state.start_status;
}

extern "C" void MockedHAL_DMA_IRQHandler_Impl(DMA_HandleTypeDef* hdma) {
    g_state.calls[static_cast<std::size_t>(ST_LIB::MockedHAL::DMAOperation::IRQHandler)]++;
    g_state.last_irq_handle = hdma;
}
