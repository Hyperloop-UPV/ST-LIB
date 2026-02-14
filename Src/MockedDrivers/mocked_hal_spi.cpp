#include "MockedDrivers/mocked_hal_spi.hpp"

#include <array>
#include <vector>

namespace {

struct SPIState {
    HAL_StatusTypeDef next_status = HAL_OK;
    bool busy = false;
    std::array<std::size_t, 9> calls{};
    std::vector<uint8_t> rx_pattern{};
    std::vector<uint8_t> last_tx{};
    std::size_t last_size_words = 0;
    SPI_HandleTypeDef* last_handle = nullptr;
};

SPIState g_state{};

void fill_rx(uint8_t* dst, std::size_t size_bytes) {
    if (dst == nullptr || size_bytes == 0) {
        return;
    }
    if (g_state.rx_pattern.empty()) {
        for (std::size_t i = 0; i < size_bytes; ++i) {
            dst[i] = 0;
        }
        return;
    }
    for (std::size_t i = 0; i < size_bytes; ++i) {
        dst[i] = g_state.rx_pattern[i % g_state.rx_pattern.size()];
    }
}

void store_tx(uint8_t* src, std::size_t size_bytes) {
    g_state.last_tx.assign(size_bytes, 0);
    if (src == nullptr || size_bytes == 0) {
        return;
    }
    for (std::size_t i = 0; i < size_bytes; ++i) {
        g_state.last_tx[i] = src[i];
    }
}

std::size_t frame_bytes(const SPI_HandleTypeDef* hspi) {
    if (hspi == nullptr) {
        return 1;
    }
    const uint32_t bits = hspi->Init.DataSize + 1U;
    if (bits <= 8U) {
        return 1;
    }
    if (bits <= 16U) {
        return 2;
    }
    return 4;
}

HAL_StatusTypeDef take_status() {
    if (g_state.busy) {
        return HAL_BUSY;
    }
    return g_state.next_status;
}

} // namespace

namespace ST_LIB::MockedHAL {

void spi_reset() { g_state = {}; }

void spi_set_status(HAL_StatusTypeDef status) { g_state.next_status = status; }

void spi_set_busy(bool busy) { g_state.busy = busy; }

void spi_set_rx_pattern(std::span<const uint8_t> pattern) {
    g_state.rx_pattern.assign(pattern.begin(), pattern.end());
}

std::size_t spi_get_call_count(SPIOperation op) {
    return g_state.calls[static_cast<std::size_t>(op)];
}

std::size_t spi_get_last_size_words() { return g_state.last_size_words; }

const uint8_t* spi_get_last_tx_data() { return g_state.last_tx.data(); }

std::size_t spi_get_last_tx_size_bytes() { return g_state.last_tx.size(); }

SPI_HandleTypeDef* spi_get_last_handle() { return g_state.last_handle; }

} // namespace ST_LIB::MockedHAL

extern "C" HAL_StatusTypeDef HAL_SPI_Init(SPI_HandleTypeDef* hspi) {
    g_state.calls[static_cast<std::size_t>(ST_LIB::MockedHAL::SPIOperation::Init)]++;
    g_state.last_handle = hspi;
    if (hspi == nullptr) {
        return HAL_ERROR;
    }
    hspi->State = HAL_SPI_STATE_READY;
    hspi->ErrorCode = HAL_SPI_ERROR_NONE;
    return take_status();
}

extern "C" HAL_StatusTypeDef HAL_SPI_Abort(SPI_HandleTypeDef* hspi) {
    g_state.calls[static_cast<std::size_t>(ST_LIB::MockedHAL::SPIOperation::Abort)]++;
    g_state.last_handle = hspi;
    if (hspi == nullptr) {
        return HAL_ERROR;
    }
    hspi->State = HAL_SPI_STATE_READY;
    return take_status();
}

extern "C" HAL_StatusTypeDef
HAL_SPI_Transmit(SPI_HandleTypeDef* hspi, uint8_t* pData, uint16_t Size, uint32_t Timeout) {
    (void)Timeout;
    g_state.calls[static_cast<std::size_t>(ST_LIB::MockedHAL::SPIOperation::Transmit)]++;
    g_state.last_handle = hspi;
    g_state.last_size_words = Size;
    store_tx(pData, static_cast<std::size_t>(Size) * frame_bytes(hspi));
    return take_status();
}

extern "C" HAL_StatusTypeDef
HAL_SPI_Receive(SPI_HandleTypeDef* hspi, uint8_t* pData, uint16_t Size, uint32_t Timeout) {
    (void)Timeout;
    g_state.calls[static_cast<std::size_t>(ST_LIB::MockedHAL::SPIOperation::Receive)]++;
    g_state.last_handle = hspi;
    g_state.last_size_words = Size;
    fill_rx(pData, static_cast<std::size_t>(Size) * frame_bytes(hspi));
    return take_status();
}

extern "C" HAL_StatusTypeDef HAL_SPI_TransmitReceive(
    SPI_HandleTypeDef* hspi,
    uint8_t* pTxData,
    uint8_t* pRxData,
    uint16_t Size,
    uint32_t Timeout
) {
    (void)Timeout;
    g_state.calls[static_cast<std::size_t>(ST_LIB::MockedHAL::SPIOperation::TransmitReceive)]++;
    g_state.last_handle = hspi;
    g_state.last_size_words = Size;
    const auto size_bytes = static_cast<std::size_t>(Size) * frame_bytes(hspi);
    store_tx(pTxData, size_bytes);
    fill_rx(pRxData, size_bytes);
    return take_status();
}

extern "C" HAL_StatusTypeDef
HAL_SPI_Transmit_DMA(SPI_HandleTypeDef* hspi, uint8_t* pData, uint16_t Size) {
    g_state.calls[static_cast<std::size_t>(ST_LIB::MockedHAL::SPIOperation::TransmitDMA)]++;
    g_state.last_handle = hspi;
    g_state.last_size_words = Size;
    store_tx(pData, static_cast<std::size_t>(Size) * frame_bytes(hspi));
    return take_status();
}

extern "C" HAL_StatusTypeDef
HAL_SPI_Receive_DMA(SPI_HandleTypeDef* hspi, uint8_t* pData, uint16_t Size) {
    g_state.calls[static_cast<std::size_t>(ST_LIB::MockedHAL::SPIOperation::ReceiveDMA)]++;
    g_state.last_handle = hspi;
    g_state.last_size_words = Size;
    fill_rx(pData, static_cast<std::size_t>(Size) * frame_bytes(hspi));
    return take_status();
}

extern "C" HAL_StatusTypeDef HAL_SPI_TransmitReceive_DMA(
    SPI_HandleTypeDef* hspi,
    uint8_t* pTxData,
    uint8_t* pRxData,
    uint16_t Size
) {
    g_state.calls[static_cast<std::size_t>(ST_LIB::MockedHAL::SPIOperation::TransmitReceiveDMA)]++;
    g_state.last_handle = hspi;
    g_state.last_size_words = Size;
    const auto size_bytes = static_cast<std::size_t>(Size) * frame_bytes(hspi);
    store_tx(pTxData, size_bytes);
    fill_rx(pRxData, size_bytes);
    return take_status();
}

extern "C" void HAL_SPI_IRQHandler(SPI_HandleTypeDef* hspi) {
    g_state.calls[static_cast<std::size_t>(ST_LIB::MockedHAL::SPIOperation::IRQHandler)]++;
    g_state.last_handle = hspi;
}
