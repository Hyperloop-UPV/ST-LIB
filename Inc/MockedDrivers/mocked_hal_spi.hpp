#pragma once

#include "hal_wrapper.h"

#include <cstddef>
#include <cstdint>
#include <span>

namespace ST_LIB::MockedHAL {

enum class SPIOperation : uint8_t {
    Init = 0,
    Abort,
    Transmit,
    Receive,
    TransmitReceive,
    TransmitDMA,
    ReceiveDMA,
    TransmitReceiveDMA,
    IRQHandler,
};

void spi_reset();

void spi_set_status(HAL_StatusTypeDef status);
void spi_set_busy(bool busy);
void spi_set_rx_pattern(std::span<const uint8_t> pattern);

std::size_t spi_get_call_count(SPIOperation op);
std::size_t spi_get_last_size_words();
const uint8_t* spi_get_last_tx_data();
std::size_t spi_get_last_tx_size_bytes();
SPI_HandleTypeDef* spi_get_last_handle();

} // namespace ST_LIB::MockedHAL
