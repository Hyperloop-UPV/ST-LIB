#include "ST-LIB_LOW/Sd/Sd.hpp"


using namespace ST_LIB;

void SdDomain::Instance::on_dma_read_complete() {
    if (operation_flag) {
        *operation_flag = true;
        operation_flag = nullptr;
    }
    SDMMC_CmdStopTransfer(hsd.Instance);
}

void SdDomain::Instance::on_dma_write_complete() {
    if (operation_flag) {
        *operation_flag = true;
        operation_flag = nullptr;
    }
    SDMMC_CmdStopTransfer(hsd.Instance);
}

void SdDomain::Instance::on_abort() { ErrorHandler("SD Card operation aborted"); }

void SdDomain::Instance::on_error() { ErrorHandler("SD Card error occurred"); }

bool SdDomain::Instance::is_card_present() { return cd_instance->first->read() == cd_instance->second; }
bool SdDomain::Instance::is_write_protected() { return wp_instance->first->read() == wp_instance->second; }

bool SdDomain::Instance::is_busy() { 
    if (!card_initialized) return false;
    return (hsd.State == HAL_SD_STATE_BUSY) || (hsd.State == HAL_SD_STATE_PROGRAMMING) || (hsd.State == HAL_SD_STATE_RECEIVING);
}

bool SdDomain::Instance::initialize_card() {
    if (card_initialized) { return true; } // Already initialized

    HAL_StatusTypeDef status = HAL_SD_Init(&hsd);
    if (status != HAL_OK) {
        return false;
    }

    if (HAL_SD_GetCardInfo(&hsd, &card_info) != HAL_OK) {
        return false;
    }

    if (!configure_idma()) {
        return false;
    }

    if (HAL_SD_ConfigSpeedBusOperation(&hsd, SDMMC_SPEED_MODE_AUTO) != HAL_OK) {
        return false;
    }

    card_initialized = true;
    return true;
}

bool SdDomain::Instance::deinitialize_card() {
    if (!card_initialized) { return true; } // Already deinitialized

    HAL_StatusTypeDef status = HAL_SD_DeInit(&hsd);
    if (status != HAL_OK) {
        return false;
    }

    card_initialized = false;
    return true;
}

void SdDomain::Instance::switch_buffer() {
    current_buffer = (current_buffer == BufferSelect::Buffer0) ? BufferSelect::Buffer1 : BufferSelect::Buffer0;
}

bool SdDomain::Instance::configure_idma() {
    HAL_StatusTypeDef status = HAL_SDEx_ConfigDMAMultiBuffer(&hsd,
        reinterpret_cast<uint32_t*>(mpu_buffer0_instance->ptr),
        reinterpret_cast<uint32_t*>(mpu_buffer1_instance->ptr),
        mpu_buffer0_instance->size / 512); // Number of 512B-blocks

    if (status != HAL_OK) { return false; }
    return true;
}

HAL_StatusTypeDef SdDomain::Instance::Not_HAL_SDEx_ReadBlocksDMAMultiBuffer(uint32_t BlockAdd, uint32_t NumberOfBlocks) {
    SDMMC_DataInitTypeDef config;
    uint32_t errorstate;
    uint32_t DmaBase0_reg;
    uint32_t DmaBase1_reg;
    uint32_t add = BlockAdd;

    if (hsd.State == HAL_SD_STATE_READY) {
        if ((add + NumberOfBlocks) > (hsd.SdCard.LogBlockNbr)) {
            hsd.ErrorCode |= HAL_SD_ERROR_ADDR_OUT_OF_RANGE;
            return HAL_ERROR;
        }

        DmaBase0_reg = hsd.Instance->IDMABASE0;
        DmaBase1_reg = hsd.Instance->IDMABASE1;

        if ((hsd.Instance->IDMABSIZE == 0U) || (DmaBase0_reg == 0U) || (DmaBase1_reg == 0U)) {
            hsd.ErrorCode = HAL_SD_ERROR_ADDR_OUT_OF_RANGE;
            return HAL_ERROR;
        }

        /* Initialize data control register */
        hsd.Instance->DCTRL = 0;
        /* Clear old Flags*/
        __HAL_SD_CLEAR_FLAG(&hsd, SDMMC_STATIC_DATA_FLAGS);

        hsd.ErrorCode = HAL_SD_ERROR_NONE;
        hsd.State = HAL_SD_STATE_BUSY;

        if (hsd.SdCard.CardType != CARD_SDHC_SDXC) {
            add *= 512U;
        }

        /* Configure the SD DPSM (Data Path State Machine) */
        config.DataTimeOut   = SDMMC_DATATIMEOUT;
        config.DataLength    = BLOCKSIZE * NumberOfBlocks;
        config.DataBlockSize = SDMMC_DATABLOCK_SIZE_512B;
        config.TransferDir   = SDMMC_TRANSFER_DIR_TO_SDMMC;
        config.TransferMode  = SDMMC_TRANSFER_MODE_BLOCK;
        config.DPSM          = SDMMC_DPSM_DISABLE;
        (void)SDMMC_ConfigData(hsd.Instance, &config);

        hsd.Instance->DCTRL |= SDMMC_DCTRL_FIFORST;

        __SDMMC_CMDTRANS_ENABLE(hsd.Instance);
        if (current_buffer == BufferSelect::Buffer1) {
            hsd.Instance->IDMACTRL = SDMMC_ENABLE_IDMA_DOUBLE_BUFF1;
        } else {
            hsd.Instance->IDMACTRL = SDMMC_ENABLE_IDMA_DOUBLE_BUFF0;
        }
        switch_buffer();

        /* Read Blocks in DMA mode */
        hsd.Context = (SD_CONTEXT_READ_MULTIPLE_BLOCK | SD_CONTEXT_DMA);

        /* Read Multi Block command */
        errorstate = SDMMC_CmdReadMultiBlock(hsd.Instance, add);
        if (errorstate != HAL_SD_ERROR_NONE) {
            hsd.State = HAL_SD_STATE_READY;
            hsd.ErrorCode |= errorstate;
            return HAL_ERROR;
        }

        __HAL_SD_ENABLE_IT(&hsd, (SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT | SDMMC_IT_RXOVERR | SDMMC_IT_DATAEND |
                                SDMMC_IT_IDMABTC));

        return HAL_OK;
    }
    else {
        return HAL_BUSY;
    }
}

HAL_StatusTypeDef SdDomain::Instance::Not_HAL_SDEx_WriteBlocksDMAMultiBuffer(uint32_t BlockAdd, uint32_t NumberOfBlocks) {
    SDMMC_DataInitTypeDef config;
    uint32_t errorstate;
    uint32_t DmaBase0_reg;
    uint32_t DmaBase1_reg;
    uint32_t add = BlockAdd;

    if (hsd.State == HAL_SD_STATE_READY) {
        if ((add + NumberOfBlocks) > (hsd.SdCard.LogBlockNbr)) {
            hsd.ErrorCode |= HAL_SD_ERROR_ADDR_OUT_OF_RANGE;
            return HAL_ERROR;
        }

        DmaBase0_reg = hsd.Instance->IDMABASE0;
        DmaBase1_reg = hsd.Instance->IDMABASE1;
        if ((hsd.Instance->IDMABSIZE == 0U) || (DmaBase0_reg == 0U) || (DmaBase1_reg == 0U)) {
            hsd.ErrorCode = HAL_SD_ERROR_ADDR_OUT_OF_RANGE;
            return HAL_ERROR;
        }

        /* Initialize data control register */
        hsd.Instance->DCTRL = 0;

        hsd.ErrorCode = HAL_SD_ERROR_NONE;

        hsd.State = HAL_SD_STATE_BUSY;
        if (hsd.SdCard.CardType != CARD_SDHC_SDXC) {
            add *= 512U;
        }

        /* Configure the SD DPSM (Data Path State Machine) */
        config.DataTimeOut   = SDMMC_DATATIMEOUT;
        config.DataLength    = BLOCKSIZE * NumberOfBlocks;
        config.DataBlockSize = SDMMC_DATABLOCK_SIZE_512B;
        config.TransferDir   = SDMMC_TRANSFER_DIR_TO_CARD;
        config.TransferMode  = SDMMC_TRANSFER_MODE_BLOCK;
        config.DPSM          = SDMMC_DPSM_DISABLE;
        (void)SDMMC_ConfigData(hsd.Instance, &config);

        __SDMMC_CMDTRANS_ENABLE(hsd.Instance);

        if (current_buffer == BufferSelect::Buffer1) {
            hsd.Instance->IDMACTRL = SDMMC_ENABLE_IDMA_DOUBLE_BUFF1;
        } else {
            hsd.Instance->IDMACTRL = SDMMC_ENABLE_IDMA_DOUBLE_BUFF0;
        }
        switch_buffer();

        /* Write Blocks in DMA mode */
        hsd.Context = (SD_CONTEXT_WRITE_MULTIPLE_BLOCK | SD_CONTEXT_DMA);

        /* Write Multi Block command */
        errorstate = SDMMC_CmdWriteMultiBlock(hsd.Instance, add);
        if (errorstate != HAL_SD_ERROR_NONE) {
            hsd.State = HAL_SD_STATE_READY;
            hsd.ErrorCode |= errorstate;
            return HAL_ERROR;
        }

        __HAL_SD_ENABLE_IT(&hsd, (SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT | SDMMC_IT_TXUNDERR | SDMMC_IT_DATAEND |
                                SDMMC_IT_IDMABTC));

        return HAL_OK;
    }
    else {
        return HAL_BUSY;
    }
}


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