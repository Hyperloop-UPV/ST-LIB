#include "ST-LIB_LOW/Sd/Sd.hpp"

// 1. Define the globals here (Strong definitions)
SD_HandleTypeDef* g_sdmmc1_handle = nullptr;
SD_HandleTypeDef* g_sdmmc2_handle = nullptr;

void* g_sdmmc1_instance_ptr = nullptr;
void* g_sdmmc2_instance_ptr = nullptr;

extern "C" {

void HAL_SD_MspInit(SD_HandleTypeDef* hsd) {
    if (hsd->Instance == SDMMC1) {
        __HAL_RCC_SDMMC1_CLK_ENABLE();
        __HAL_RCC_SDMMC1_FORCE_RESET();
        __HAL_RCC_SDMMC1_RELEASE_RESET();
        HAL_NVIC_SetPriority(SDMMC1_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(SDMMC1_IRQn);
    } else {
        __HAL_RCC_SDMMC2_CLK_ENABLE();
        __HAL_RCC_SDMMC2_FORCE_RESET();
        __HAL_RCC_SDMMC2_RELEASE_RESET();
        HAL_NVIC_SetPriority(SDMMC2_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(SDMMC2_IRQn);
    }
}

void SDMMC1_IRQHandler(void) {
    if (g_sdmmc1_handle != nullptr) {
        HAL_SD_IRQHandler(g_sdmmc1_handle);
    }
}

void SDMMC2_IRQHandler(void) {
    if (g_sdmmc2_handle != nullptr) {
        HAL_SD_IRQHandler(g_sdmmc2_handle);
    }
}

void HAL_SDEx_Read_DMADoubleBuf0CpltCallback(SD_HandleTypeDef* hsd) {
    if (auto sd_instance = ST_LIB::get_sd_instance(hsd)) {
        sd_instance->on_dma_read_complete();
    }
}

void HAL_SDEx_Read_DMADoubleBuf1CpltCallback(SD_HandleTypeDef* hsd) {
    if (auto sd_instance = ST_LIB::get_sd_instance(hsd)) {
        sd_instance->on_dma_read_complete();
    }
}

void HAL_SDEx_Write_DMADoubleBuf0CpltCallback(SD_HandleTypeDef* hsd) {
    if (auto sd_instance = ST_LIB::get_sd_instance(hsd)) {
        sd_instance->on_dma_write_complete();
    }
}

void HAL_SDEx_Write_DMADoubleBuf1CpltCallback(SD_HandleTypeDef* hsd) {
    if (auto sd_instance = ST_LIB::get_sd_instance(hsd)) {
        sd_instance->on_dma_write_complete();
    }
}

void HAL_SD_AbortCallback(SD_HandleTypeDef* hsd) {
    if (auto sd_instance = ST_LIB::get_sd_instance(hsd)) {
        sd_instance->on_abort();
    } else {
        ErrorHandler("SD Card operation aborted");
    }
}

void HAL_SD_ErrorCallback(SD_HandleTypeDef* hsd) {
    if (auto sd_instance = ST_LIB::get_sd_instance(hsd)) {
        sd_instance->on_error();
    } else {
        ErrorHandler("SD Card error occurred");
    }
}

} // extern "C"