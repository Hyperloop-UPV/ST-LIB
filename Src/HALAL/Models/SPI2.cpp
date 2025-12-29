#include "HALAL/Models/SPI2.hpp"

ST_LIB::SPIDomain::Instance* spi_instances[ST_LIB::SPIDomain::max_instances];

extern "C" {

/**
 * =========================================
 *               IRQ Handlers
 * =========================================
 */

void SPI1_IRQHandler(void) {
    auto inst = ST_LIB::SPIDomain::spi_instances[0];
    if (inst == nullptr) {
        ErrorHandler("SPI1 IRQ Handler called but instance is null");
        return;
    }
    HAL_SPI_IRQHandler(&inst->hspi);
}
void SPI2_IRQHandler(void) {
    auto inst = ST_LIB::SPIDomain::spi_instances[1];
    if (inst == nullptr) {
        ErrorHandler("SPI2 IRQ Handler called but instance is null");
        return;
    }
    HAL_SPI_IRQHandler(&inst->hspi);
}
void SPI3_IRQHandler(void) {
    auto inst = ST_LIB::SPIDomain::spi_instances[2];
    if (inst == nullptr) {
        ErrorHandler("SPI3 IRQ Handler called but instance is null");
        return;
    }
    HAL_SPI_IRQHandler(&inst->hspi);
}
void SPI4_IRQHandler(void) {
    auto inst = ST_LIB::SPIDomain::spi_instances[3];
    if (inst == nullptr) {
        ErrorHandler("SPI4 IRQ Handler called but instance is null");
        return;
    }
    HAL_SPI_IRQHandler(&inst->hspi);
}
void SPI5_IRQHandler(void) {
    auto inst = ST_LIB::SPIDomain::spi_instances[4];
    if (inst == nullptr) {
        ErrorHandler("SPI5 IRQ Handler called but instance is null");
        return;
    }
    HAL_SPI_IRQHandler(&inst->hspi);
}
void SPI6_IRQHandler(void) {
    auto inst = ST_LIB::SPIDomain::spi_instances[5];
    if (inst == nullptr) {
        ErrorHandler("SPI6 IRQ Handler called but instance is null");
        return;
    }
    HAL_SPI_IRQHandler(&inst->hspi);
}


/**
 * =========================================
 *               HAL Callbacks
 * =========================================
 */

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
    ST_LIB::SPIDomain::Instance* inst = nullptr;
    if (spi_instances[0] != nullptr && hspi == &spi_instances[0]->hspi) {
        inst = spi_instances[0];
    } else if (spi_instances[1] != nullptr && hspi == &spi_instances[1]->hspi) {
        inst = spi_instances[1];
    } else if (spi_instances[2] != nullptr && hspi == &spi_instances[2]->hspi) {
        inst = spi_instances[2];
    } else if (spi_instances[3] != nullptr && hspi == &spi_instances[3]->hspi) {
        inst = spi_instances[3];
    } else if (spi_instances[4] != nullptr && hspi == &spi_instances[4]->hspi) {
        inst = spi_instances[4];
    } else if (spi_instances[5] != nullptr && hspi == &spi_instances[5]->hspi) {
        inst = spi_instances[5];
    } else {
        ErrorHandler("SPI IRQ Callback called but instance is null");
        return;
    }

    if (inst->operation_flag != nullptr) {
        *(inst->operation_flag) = true;
    }
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {
    HAL_SPI_TxCpltCallback(hspi); // Same logic
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
    HAL_SPI_TxCpltCallback(hspi); // Same logic
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi) {
    uint32_t error_code = hspi->ErrorCode;
    uint32_t inst_idx = 0;
    if (spi_instances[0] != nullptr && hspi == &spi_instances[0]->hspi) {
        inst_idx = 0;
    } else if (spi_instances[1] != nullptr && hspi == &spi_instances[1]->hspi) {
        inst_idx = 1;
    } else if (spi_instances[2] != nullptr && hspi == &spi_instances[2]->hspi) {
        inst_idx = 2;
    } else if (spi_instances[3] != nullptr && hspi == &spi_instances[3]->hspi) {
        inst_idx = 3;
    } else if (spi_instances[4] != nullptr && hspi == &spi_instances[4]->hspi) {
        inst_idx = 4;
    } else if (spi_instances[5] != nullptr && hspi == &spi_instances[5]->hspi) {
        inst_idx = 5;
    } else {
        ErrorHandler("SPI IRQ Callback called but instance is null");
        return;
    }

    ErrorHandler("SPI%i failed with error number %u", inst_idx + 1, error_code);
}

}