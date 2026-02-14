/*
 * SPI2.hpp
 *
 *  Created on: 27 dec. 2025
 *      Author: Boris
 */

#ifndef SPI2_HPP
#define SPI2_HPP

#include "hal_wrapper.h"
#include "C++Utilities/CppImports.hpp"
#include "HALAL/Models/GPIO.hpp"
#include "HALAL/Models/Pin.hpp"
#include "ErrorHandler/ErrorHandler.hpp"
#include "HALAL/Models/DMA/DMA2.hpp"
#include "HALAL/Models/SPI/SPIConfig.hpp"

using ST_LIB::DMA_Domain;
using ST_LIB::GPIODomain;
using ST_LIB::SPIConfigTypes;

// Forward declaration of IRQ handlers and HAL callbacks
extern "C" {
void SPI1_IRQHandler(void);
void SPI2_IRQHandler(void);
void SPI3_IRQHandler(void);
void SPI4_IRQHandler(void);
void SPI5_IRQHandler(void);
void SPI6_IRQHandler(void);

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef* hspi);
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef* hspi);
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef* hspi);
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef* hspi);
}

namespace ST_LIB {
extern void compile_error(const char* msg);

struct SPIDomain {

    /**
     * =========================================
     *          Internal working things
     * =========================================
     */

    // Configuration types
    using ClockPolarity = SPIConfigTypes::ClockPolarity;
    using ClockPhase = SPIConfigTypes::ClockPhase;
    using BitOrder = SPIConfigTypes::BitOrder;
    using NSSMode = SPIConfigTypes::NSSMode;
    using DataSize = SPIConfigTypes::DataSize;
    using Direction = SPIConfigTypes::Direction;
    using FIFOThreshold = SPIConfigTypes::FIFOThreshold;
    using NSSPolarity = SPIConfigTypes::NSSPolarity;
    using CRCLength = SPIConfigTypes::CRCLength;
    using SPIConfig = SPIConfigTypes::SPIConfig;

    enum class SPIPeripheral : uintptr_t {
        spi1 = SPI1_BASE,
        spi2 = SPI2_BASE,
        spi3 = SPI3_BASE,
        spi4 = SPI4_BASE,
        spi5 = SPI5_BASE,
        spi6 = SPI6_BASE,
    };

    enum class SPIMode : bool {
        MASTER = true,
        SLAVE = false,
    };

    static consteval bool compare_pin(const GPIODomain::Pin& p1, const GPIODomain::Pin& p2) {
        return (p1.port == p2.port) && (p1.pin == p2.pin);
    }

    static consteval GPIODomain::AlternateFunction
    get_af(const GPIODomain::Pin& pin, SPIPeripheral peripheral) {

        if (peripheral == SPIPeripheral::spi2) {
            if (compare_pin(pin, PB4))
                return GPIODomain::AlternateFunction::AF7;
        }
        if (peripheral == SPIPeripheral::spi3) {
            if (compare_pin(pin, PA4))
                return GPIODomain::AlternateFunction::AF6;
            if (compare_pin(pin, PA15))
                return GPIODomain::AlternateFunction::AF6;
            if (compare_pin(pin, PB2))
                return GPIODomain::AlternateFunction::AF7;
            if (compare_pin(pin, PB3))
                return GPIODomain::AlternateFunction::AF6;
            if (compare_pin(pin, PB4))
                return GPIODomain::AlternateFunction::AF6;
            if (compare_pin(pin, PB5))
                return GPIODomain::AlternateFunction::AF7;
            if (compare_pin(pin, PC10))
                return GPIODomain::AlternateFunction::AF6;
            if (compare_pin(pin, PC11))
                return GPIODomain::AlternateFunction::AF6;
            if (compare_pin(pin, PC12))
                return GPIODomain::AlternateFunction::AF6;
        }
        if (peripheral == SPIPeripheral::spi6) {
            if (compare_pin(pin, PA4))
                return GPIODomain::AlternateFunction::AF8;
            if (compare_pin(pin, PA5))
                return GPIODomain::AlternateFunction::AF8;
            if (compare_pin(pin, PA6))
                return GPIODomain::AlternateFunction::AF8;
            if (compare_pin(pin, PA7))
                return GPIODomain::AlternateFunction::AF8;
            if (compare_pin(pin, PA15))
                return GPIODomain::AlternateFunction::AF7;
            if (compare_pin(pin, PB3))
                return GPIODomain::AlternateFunction::AF8;
            if (compare_pin(pin, PB4))
                return GPIODomain::AlternateFunction::AF8;
            if (compare_pin(pin, PB5))
                return GPIODomain::AlternateFunction::AF8;
        }

        return GPIODomain::AlternateFunction::AF5; // Default AF for everything else
    }

    static constexpr uint32_t get_prescaler_flag(uint32_t prescaler) {
        switch (prescaler) {
        case 2:
            return SPI_BAUDRATEPRESCALER_2;
        case 4:
            return SPI_BAUDRATEPRESCALER_4;
        case 8:
            return SPI_BAUDRATEPRESCALER_8;
        case 16:
            return SPI_BAUDRATEPRESCALER_16;
        case 32:
            return SPI_BAUDRATEPRESCALER_32;
        case 64:
            return SPI_BAUDRATEPRESCALER_64;
        case 128:
            return SPI_BAUDRATEPRESCALER_128;
        case 256:
            return SPI_BAUDRATEPRESCALER_256;
        default:
            if consteval {
                compile_error("Invalid prescaler value");
            } else {
                ErrorHandler("Invalid prescaler value");
                return SPI_BAUDRATEPRESCALER_256;
            }
        }
    }

    // Forward declaration
    static uint32_t calculate_prescaler(uint32_t src_freq, uint32_t max_baud);

    static constexpr std::size_t max_instances{6};

    struct Entry {
        SPIPeripheral peripheral;
        SPIMode mode;

        std::size_t sck_gpio_idx;
        std::size_t miso_gpio_idx;
        std::size_t mosi_gpio_idx;
        std::optional<std::size_t> nss_gpio_idx;

        std::size_t dma_rx_idx;
        std::size_t dma_tx_idx;

        uint32_t max_baudrate; // Will set the baudrate as fast as possible under this value
        SPIConfig config;      // User-defined SPI configuration
    };

    struct Config {
        SPIPeripheral peripheral;
        SPIMode mode;

        std::size_t sck_gpio_idx;
        std::size_t miso_gpio_idx;
        std::size_t mosi_gpio_idx;
        std::optional<std::size_t> nss_gpio_idx;

        std::size_t dma_rx_idx;
        std::size_t dma_tx_idx;

        uint32_t max_baudrate; // Will set the baudrate as fast as possible under this value
        SPIConfig config;      // User-defined SPI configuration
    };

    /**
     * =========================================
     *              Request Object
     * =========================================
     */
    template <DMA_Domain::Stream dma_rx_stream, DMA_Domain::Stream dma_tx_stream> struct Device {
        using domain = SPIDomain;

        SPIPeripheral peripheral;
        SPIMode mode;
        uint32_t max_baudrate; // Will set the baudrate as fast as possible under this value
        SPIConfig config;      // User-defined SPI configuration

        GPIODomain::GPIO sck_gpio;
        GPIODomain::GPIO miso_gpio;
        GPIODomain::GPIO mosi_gpio;
        std::optional<GPIODomain::GPIO> nss_gpio;

        DMA_Domain::DMA<dma_rx_stream, dma_tx_stream> dma_rx_tx;

        consteval Device(
            SPIMode mode,
            SPIPeripheral peripheral,
            uint32_t max_baudrate,
            GPIODomain::Pin sck_pin,
            GPIODomain::Pin miso_pin,
            GPIODomain::Pin mosi_pin,
            GPIODomain::Pin nss_pin,
            SPIConfig config = SPIConfig{}
        )
            : peripheral{peripheral}, mode{mode}, max_baudrate{max_baudrate}, config{config},
              sck_gpio(
                  sck_pin,
                  GPIODomain::OperationMode::ALT_PP,
                  GPIODomain::Pull::None,
                  GPIODomain::Speed::VeryHigh,
                  get_af(sck_pin, peripheral)
              ),
              miso_gpio(
                  miso_pin,
                  GPIODomain::OperationMode::ALT_PP,
                  GPIODomain::Pull::None,
                  GPIODomain::Speed::VeryHigh,
                  get_af(miso_pin, peripheral)
              ),
              mosi_gpio(
                  mosi_pin,
                  GPIODomain::OperationMode::ALT_PP,
                  GPIODomain::Pull::None,
                  GPIODomain::Speed::VeryHigh,
                  get_af(mosi_pin, peripheral)
              ),
              nss_gpio(GPIODomain::GPIO(
                  nss_pin,
                  GPIODomain::OperationMode::ALT_PP,
                  GPIODomain::Pull::None,
                  GPIODomain::Speed::VeryHigh,
                  get_af(nss_pin, peripheral)
              )),
              dma_rx_tx(dma_peripheral(peripheral)) {
            config.validate();

            if (config.nss_mode == NSSMode::SOFTWARE) {
                compile_error("Use NSSMode::SOFTWARE, and omit NSS pin for software NSS "
                              "management, it is handled externally");
            }

            validate_nss_pin(peripheral, nss_pin);

            validate_spi_pins(peripheral, sck_pin, miso_pin, mosi_pin);
        }

        // Constructor without NSS pin (for software NSS mode)
        consteval Device(
            SPIMode mode,
            SPIPeripheral peripheral,
            uint32_t max_baudrate,
            GPIODomain::Pin sck_pin,
            GPIODomain::Pin miso_pin,
            GPIODomain::Pin mosi_pin,
            SPIConfig config
        )
            : peripheral{peripheral}, mode{mode}, max_baudrate{max_baudrate}, config{config},
              sck_gpio(
                  sck_pin,
                  GPIODomain::OperationMode::ALT_PP,
                  GPIODomain::Pull::None,
                  GPIODomain::Speed::VeryHigh,
                  get_af(sck_pin, peripheral)
              ),
              miso_gpio(
                  miso_pin,
                  GPIODomain::OperationMode::ALT_PP,
                  GPIODomain::Pull::None,
                  GPIODomain::Speed::VeryHigh,
                  get_af(miso_pin, peripheral)
              ),
              mosi_gpio(
                  mosi_pin,
                  GPIODomain::OperationMode::ALT_PP,
                  GPIODomain::Pull::None,
                  GPIODomain::Speed::VeryHigh,
                  get_af(mosi_pin, peripheral)
              ),
              nss_gpio(std::nullopt), // No NSS GPIO
              dma_rx_tx(dma_peripheral(peripheral)) {
            config.validate();

            if (config.nss_mode == NSSMode::HARDWARE) {
                compile_error(
                    "NSS pin must be provided for hardware NSS mode, or use NSSMode::SOFTWARE"
                );
            }

            validate_spi_pins(peripheral, sck_pin, miso_pin, mosi_pin);
        }

        template <class Ctx> consteval std::size_t inscribe(Ctx& ctx) const {
            auto dma_indices = dma_rx_tx.inscribe(ctx);

            // Conditionally add NSS GPIO if provided
            std::optional<std::size_t> nss_idx = std::nullopt;
            if (nss_gpio.has_value()) {
                nss_idx = nss_gpio.value().inscribe(ctx);
            }

            Entry e{
                .peripheral = peripheral,
                .mode = mode,
                .sck_gpio_idx = sck_gpio.inscribe(ctx),
                .miso_gpio_idx = miso_gpio.inscribe(ctx),
                .mosi_gpio_idx = mosi_gpio.inscribe(ctx),
                .nss_gpio_idx = nss_idx,
                .dma_rx_idx = dma_indices[0],
                .dma_tx_idx = dma_indices[1],
                .max_baudrate = max_baudrate,
                .config = config
            };

            return ctx.template add<SPIDomain>(e, this);
        }

    private:
        // Helper function to validate SPI pins (SCK, MISO, MOSI only)
        static consteval void validate_spi_pins(
            SPIPeripheral peripheral,
            GPIODomain::Pin sck_pin,
            GPIODomain::Pin miso_pin,
            GPIODomain::Pin mosi_pin
        ) {
            switch (peripheral) {
            case SPIPeripheral::spi1:
                if (!compare_pin(sck_pin, PB3) && !compare_pin(sck_pin, PG11) &&
                    !compare_pin(sck_pin, PA5)) {
                    compile_error("Invalid SCK pin for SPI1");
                }
                if (!compare_pin(miso_pin, PB4) && !compare_pin(miso_pin, PG9) &&
                    !compare_pin(miso_pin, PA6)) {
                    compile_error("Invalid MISO pin for SPI1");
                }
                if (!compare_pin(mosi_pin, PB5) && !compare_pin(mosi_pin, PD7) &&
                    !compare_pin(mosi_pin, PA7)) {
                    compile_error("Invalid MOSI pin for SPI1");
                }
                break;

            case SPIPeripheral::spi2:
                if (!compare_pin(sck_pin, PD3) && !compare_pin(sck_pin, PA12) &&
                    !compare_pin(sck_pin, PA9) && !compare_pin(sck_pin, PB13) &&
                    !compare_pin(sck_pin, PB10)) {
                    compile_error("Invalid SCK pin for SPI2");
                }
                if (!compare_pin(miso_pin, PC2) && !compare_pin(miso_pin, PB14)) {
                    compile_error("Invalid MISO pin for SPI2");
                }
                if (!compare_pin(mosi_pin, PC3) && !compare_pin(mosi_pin, PC1) &&
                    !compare_pin(mosi_pin, PB15)) {
                    compile_error("Invalid MOSI pin for SPI2");
                }
                break;

            case SPIPeripheral::spi3:
                if (!compare_pin(sck_pin, PB3) && !compare_pin(sck_pin, PC10)) {
                    compile_error("Invalid SCK pin for SPI3");
                }
                if (!compare_pin(miso_pin, PB4) && !compare_pin(miso_pin, PC11)) {
                    compile_error("Invalid MISO pin for SPI3");
                }
                if (!compare_pin(mosi_pin, PB5) && !compare_pin(mosi_pin, PD6) &&
                    !compare_pin(mosi_pin, PC12) && !compare_pin(mosi_pin, PB2)) {
                    compile_error("Invalid MOSI pin for SPI3");
                }
                break;

            case SPIPeripheral::spi4:
                if (!compare_pin(sck_pin, PE2) && !compare_pin(sck_pin, PE12)) {
                    compile_error("Invalid SCK pin for SPI4");
                }
                if (!compare_pin(miso_pin, PE5) && !compare_pin(miso_pin, PE13)) {
                    compile_error("Invalid MISO pin for SPI4");
                }
                if (!compare_pin(mosi_pin, PE6) && !compare_pin(mosi_pin, PE14)) {
                    compile_error("Invalid MOSI pin for SPI4");
                }
                break;

            case SPIPeripheral::spi5:
                if (!compare_pin(sck_pin, PF7)) {
                    compile_error("Invalid SCK pin for SPI5");
                }
                if (!compare_pin(miso_pin, PF8)) {
                    compile_error("Invalid MISO pin for SPI5");
                }
                if (!compare_pin(mosi_pin, PF9) && !compare_pin(mosi_pin, PF11)) {
                    compile_error("Invalid MOSI pin for SPI5");
                }
                break;

            case SPIPeripheral::spi6:
                if (!compare_pin(sck_pin, PB3) && !compare_pin(sck_pin, PG13) &&
                    !compare_pin(sck_pin, PC10) && !compare_pin(sck_pin, PA7)) {
                    compile_error("Invalid SCK pin for SPI6");
                }
                if (!compare_pin(miso_pin, PB4) && !compare_pin(miso_pin, PG12) &&
                    !compare_pin(miso_pin, PA6)) {
                    compile_error("Invalid MISO pin for SPI6");
                }
                if (!compare_pin(mosi_pin, PB5) && !compare_pin(mosi_pin, PG14) &&
                    !compare_pin(mosi_pin, PA7)) {
                    compile_error("Invalid MOSI pin for SPI6");
                }
                break;

            default:
                compile_error("Invalid SPI peripheral specified in SPIDomain::Device");
            }
        }

        // Helper function to validate NSS pin (only called for hardware NSS mode)
        static consteval void validate_nss_pin(SPIPeripheral peripheral, GPIODomain::Pin nss_pin) {
            switch (peripheral) {
            case SPIPeripheral::spi1:
                if (!compare_pin(nss_pin, PA4) && !compare_pin(nss_pin, PA15) &&
                    !compare_pin(nss_pin, PG10)) {
                    compile_error("Invalid NSS pin for SPI1");
                }
                break;

            case SPIPeripheral::spi2:
                if (!compare_pin(nss_pin, PA11) && !compare_pin(nss_pin, PB9) &&
                    !compare_pin(nss_pin, PB4) && !compare_pin(nss_pin, PB12)) {
                    compile_error("Invalid NSS pin for SPI2");
                }
                break;

            case SPIPeripheral::spi3:
                if (!compare_pin(nss_pin, PA15) && !compare_pin(nss_pin, PA4)) {
                    compile_error("Invalid NSS pin for SPI3");
                }
                break;

            case SPIPeripheral::spi4:
                if (!compare_pin(nss_pin, PE4) && !compare_pin(nss_pin, PE11)) {
                    compile_error("Invalid NSS pin for SPI4");
                }
                break;

            case SPIPeripheral::spi5:
                if (!compare_pin(nss_pin, PF6)) {
                    compile_error("Invalid NSS pin for SPI5");
                }
                break;

            case SPIPeripheral::spi6:
                if (!compare_pin(nss_pin, PA0) && !compare_pin(nss_pin, PA15) &&
                    !compare_pin(nss_pin, PG8) && !compare_pin(nss_pin, PA4)) {
                    compile_error("Invalid NSS pin for SPI6");
                }
                break;

            default:
                compile_error("Invalid SPI peripheral specified in SPIDomain::Device");
            }
        }

        static consteval DMA_Domain::Peripheral dma_peripheral(SPIPeripheral peripheral) {
            switch (peripheral) {
            case SPIPeripheral::spi1:
                return DMA_Domain::Peripheral::spi1;
            case SPIPeripheral::spi2:
                return DMA_Domain::Peripheral::spi2;
            case SPIPeripheral::spi3:
                return DMA_Domain::Peripheral::spi3;
            case SPIPeripheral::spi4:
                return DMA_Domain::Peripheral::spi4;
            case SPIPeripheral::spi5:
                return DMA_Domain::Peripheral::spi5;
            case SPIPeripheral::spi6:
                return DMA_Domain::Peripheral::spi6;
            default:
                compile_error("Invalid SPI peripheral specified in SPIDomain::Device");
            }
        }
    };

    /**
     * =========================================
     *        Instance (state holder)
     * =========================================
     */
    template <std::size_t N> struct Init;                             // Forward declaration
    template <auto& device_request, bool IsMaster> struct SPIWrapper; // Forward declaration
    struct Instance {
        template <std::size_t> friend struct Init;
        template <auto&, bool> friend struct SPIWrapper;
        friend void ::SPI1_IRQHandler(void);
        friend void ::SPI2_IRQHandler(void);
        friend void ::SPI3_IRQHandler(void);
        friend void ::SPI4_IRQHandler(void);
        friend void ::SPI5_IRQHandler(void);
        friend void ::SPI6_IRQHandler(void);
        friend void ::HAL_SPI_TxCpltCallback(SPI_HandleTypeDef* hspi);
        friend void ::HAL_SPI_RxCpltCallback(SPI_HandleTypeDef* hspi);
        friend void ::HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef* hspi);
        friend void ::HAL_SPI_ErrorCallback(SPI_HandleTypeDef* hspi);

    private:
        SPI_HandleTypeDef hspi;
        SPI_TypeDef* instance;

        volatile bool* operation_flag = nullptr;
        volatile uint32_t error_count = 0;
        volatile bool was_aborted = false;

        bool recover() {
            // Abort any ongoing SPI operation
            HAL_SPI_Abort(&hspi);

            uint32_t newErrorCount = error_count + 1; // Cause volatile in c++ freacking sucks
            error_count = newErrorCount;
            was_aborted = true;
            operation_flag = nullptr;

            // Reset SPI state
            auto status = HAL_SPI_DeInit(&hspi);
            if (status != HAL_OK) {
                return false;
            }
            status = HAL_SPI_Init(&hspi);
            if (status != HAL_OK) {
                return false;
            }
            return true;
        }
    };

    static inline Instance* spi_instances[max_instances];
    /**
     * =========================================
     *          Wrapper, public API
     * =========================================
     */

    // SPI Wrapper primary template
    template <auto& device_request, bool IsMaster = (device_request.mode == SPIMode::MASTER)>
    struct SPIWrapper;

    /**
     * @brief SPI Wrapper for Master mode operations.
     */
    template <auto& device_request> struct SPIWrapper<device_request, true> {
        static constexpr uint32_t data_bits =
            static_cast<uint32_t>(device_request.config.data_size);
        static constexpr uint32_t frame_size = (data_bits <= 8) ? 1 : ((data_bits <= 16) ? 2 : 4);

        SPIWrapper(Instance& instance) : spi_instance{instance} {}

        /**
         * @brief Aborts any ongoing SPI operation and recovers the peripheral.
         */
        void abort_and_recover() { spi_instance.recover(); }

        /**
         * @brief Checks if the SPI instance was aborted since the last reset.
         */
        bool was_aborted() const { return spi_instance.was_aborted; }

        void clear_abort_flag() { spi_instance.was_aborted = false; }

        /**
         * @brief Gets the error count for this SPI instance.
         */
        uint32_t get_error_count() const { return spi_instance.error_count; }

        /**
         * @brief Sends data over SPI in blocking mode.
         */
        template <typename E, size_t S> bool send(span<E, S> data) {
            if (data.size_bytes() % frame_size != 0) {
                ErrorHandler(
                    "SPI data size (%d) not aligned to frame size (%d)",
                    data.size_bytes(),
                    frame_size
                );
                return false;
            }
            auto error_code = HAL_SPI_Transmit(
                &spi_instance.hspi,
                (uint8_t*)data.data(),
                data.size_bytes() / frame_size,
                10
            );
            return check_error_code(error_code);
        }

        /**
         * @brief Sends a trivially copyable data type over SPI in blocking mode.
         */
        template <typename T>
        bool send(const T& data)
            requires std::is_trivially_copyable_v<T>
        {
            if (sizeof(T) % frame_size != 0) {
                ErrorHandler(
                    "SPI data type size (%d) not aligned to frame size (%d)",
                    sizeof(T),
                    frame_size
                );
                return false;
            }
            auto error_code = HAL_SPI_Transmit(
                &spi_instance.hspi,
                reinterpret_cast<const uint8_t*>(&data),
                sizeof(T) / frame_size,
                10
            );
            return check_error_code(error_code);
        }

        /**
         * @brief Receives data over SPI in blocking mode.
         */
        template <typename E, size_t S> bool receive(span<E, S> data) {
            if (data.size_bytes() % frame_size != 0) {
                ErrorHandler(
                    "SPI data size (%d) not aligned to frame size (%d)",
                    data.size_bytes(),
                    frame_size
                );
                return false;
            }
            auto error_code = HAL_SPI_Receive(
                &spi_instance.hspi,
                (uint8_t*)data.data(),
                data.size_bytes() / frame_size,
                10
            );
            return check_error_code(error_code);
        }

        /**
         * @brief Receives a trivially copyable data type over SPI in blocking mode.
         */
        template <typename T>
        bool receive(T& data)
            requires std::is_trivially_copyable_v<T>
        {
            if (sizeof(T) % frame_size != 0) {
                ErrorHandler(
                    "SPI data type size (%d) not aligned to frame size (%d)",
                    sizeof(T),
                    frame_size
                );
                return false;
            }
            auto error_code = HAL_SPI_Receive(
                &spi_instance.hspi,
                reinterpret_cast<uint8_t*>(&data),
                sizeof(T) / frame_size,
                10
            );
            return check_error_code(error_code);
        }

        /**
         * @brief Sends and receives data over SPI in blocking mode.
         */
        template <typename E1, size_t S1, typename E2, size_t S2>
        bool transceive(span<E1, S1> tx_data, span<E2, S2> rx_data) {
            size_t size = std::min(tx_data.size_bytes(), rx_data.size_bytes());
            if (size % frame_size != 0) {
                ErrorHandler(
                    "SPI transaction size (%d) not aligned to frame size (%d)",
                    size,
                    frame_size
                );
                return false;
            }
            auto error_code = HAL_SPI_TransmitReceive(
                &spi_instance.hspi,
                (uint8_t*)tx_data.data(),
                (uint8_t*)rx_data.data(),
                size / frame_size,
                10
            );
            return check_error_code(error_code);
        }

        /**
         * @brief Sends and receives a trivially copyable data type over SPI in blocking mode.
         */
        template <typename E, size_t S, typename T>
        bool transceive(span<E, S> tx_data, T& rx_data)
            requires std::is_trivially_copyable_v<T>
        {
            size_t size = std::min(tx_data.size_bytes(), sizeof(T));
            if (size % frame_size != 0) {
                ErrorHandler(
                    "SPI transaction size (%d) not aligned to frame size (%d)",
                    size,
                    frame_size
                );
                return false;
            }
            auto error_code = HAL_SPI_TransmitReceive(
                &spi_instance.hspi,
                (uint8_t*)tx_data.data(),
                reinterpret_cast<uint8_t*>(&rx_data),
                size / frame_size,
                10
            );
            return check_error_code(error_code);
        }

        /**
         * @brief Sends and receives a trivially copyable data type over SPI in blocking mode.
         */
        template <typename T, typename E, size_t S>
        bool transceive(const T& tx_data, span<E, S> rx_data)
            requires std::is_trivially_copyable_v<T>
        {
            size_t size = std::min(sizeof(T), rx_data.size_bytes());
            if (size % frame_size != 0) {
                ErrorHandler(
                    "SPI transaction size (%d) not aligned to frame size (%d)",
                    size,
                    frame_size
                );
                return false;
            }
            auto error_code = HAL_SPI_TransmitReceive(
                &spi_instance.hspi,
                reinterpret_cast<const uint8_t*>(&tx_data),
                (uint8_t*)rx_data.data(),
                size / frame_size,
                10
            );
            return check_error_code(error_code);
        }

        /**
         * @brief Sends and receives a trivially copyable data type over SPI in blocking mode.
         */
        template <typename T1, typename T2>
        bool transceive(const T1& tx_data, T2& rx_data)
            requires(std::is_trivially_copyable_v<T1> && std::is_trivially_copyable_v<T2>)
        {
            size_t size = std::min(sizeof(T1), sizeof(T2));
            if (size % frame_size != 0) {
                ErrorHandler(
                    "SPI transaction size (%d) not aligned to frame size (%d)",
                    size,
                    frame_size
                );
                return false;
            }
            auto error_code = HAL_SPI_TransmitReceive(
                &spi_instance.hspi,
                reinterpret_cast<const uint8_t*>(&tx_data),
                reinterpret_cast<uint8_t*>(&rx_data),
                size / frame_size,
                10
            );
            return check_error_code(error_code);
        }

        /**
         * @brief Sends data over SPI using DMA, uses an optional operation flag to signal
         * completion.
         */
        template <typename E, size_t S>
        bool send_DMA(span<E, S> data, volatile bool* operation_flag = nullptr) {
            spi_instance.operation_flag = operation_flag;
            if (data.size_bytes() % frame_size != 0) {
                ErrorHandler(
                    "SPI data size (%d) not aligned to frame size (%d)",
                    data.size_bytes(),
                    frame_size
                );
                return false;
            }
            auto error_code = HAL_SPI_Transmit_DMA(
                &spi_instance.hspi,
                (uint8_t*)data.data(),
                data.size_bytes() / frame_size
            );
            return check_error_code(error_code);
        }

        /**
         * @brief Sends a trivially copyable data type over SPI using DMA, uses an optional
         * operation flag to signal completion.
         */
        template <typename T>
        bool send_DMA(const T& data, volatile bool* operation_flag = nullptr)
            requires std::is_trivially_copyable_v<T>
        {
            spi_instance.operation_flag = operation_flag;
            if (sizeof(T) % frame_size != 0) {
                ErrorHandler(
                    "SPI data size (%d) not aligned to frame size (%d)",
                    sizeof(T),
                    frame_size
                );
                return false;
            }
            auto error_code = HAL_SPI_Transmit_DMA(
                &spi_instance.hspi,
                reinterpret_cast<const uint8_t*>(&data),
                sizeof(T) / frame_size
            );
            return check_error_code(error_code);
        }

        /**
         * @brief Receives data over SPI using DMA, uses an optional operation flag to signal
         * completion.
         */
        template <typename E, size_t S>
        bool receive_DMA(span<E, S> data, volatile bool* operation_flag = nullptr) {
            spi_instance.operation_flag = operation_flag;
            if (data.size_bytes() % frame_size != 0) {
                ErrorHandler(
                    "SPI data size (%d) not aligned to frame size (%d)",
                    data.size_bytes(),
                    frame_size
                );
                return false;
            }
            auto error_code = HAL_SPI_Receive_DMA(
                &spi_instance.hspi,
                (uint8_t*)data.data(),
                data.size_bytes() / frame_size
            );
            return check_error_code(error_code);
        }

        /**
         * @brief Receives a trivially copyable data type over SPI using DMA, uses an optional
         * operation flag to signal completion.
         */
        template <typename T>
        bool receive_DMA(T& data, volatile bool* operation_flag = nullptr)
            requires std::is_trivially_copyable_v<T>
        {
            spi_instance.operation_flag = operation_flag;
            if (sizeof(T) % frame_size != 0) {
                ErrorHandler(
                    "SPI data size (%d) not aligned to frame size (%d)",
                    sizeof(T),
                    frame_size
                );
                return false;
            }
            auto error_code = HAL_SPI_Receive_DMA(
                &spi_instance.hspi,
                reinterpret_cast<uint8_t*>(&data),
                sizeof(T) / frame_size
            );
            return check_error_code(error_code);
        }

        /**
         * @brief Sends and receives data over SPI using DMA, uses an optional operation flag to
         * signal completion.
         */
        template <typename E1, size_t S1, typename E2, size_t S2>
        bool transceive_DMA(
            span<E1, S1> tx_data,
            span<E2, S2> rx_data,
            volatile bool* operation_flag = nullptr
        ) {
            spi_instance.operation_flag = operation_flag;
            auto size = std::min(tx_data.size_bytes(), rx_data.size_bytes());
            if (size % frame_size != 0) {
                ErrorHandler(
                    "SPI transaction size (%d) not aligned to frame size (%d)",
                    size,
                    frame_size
                );
                return false;
            }
            auto error_code = HAL_SPI_TransmitReceive_DMA(
                &spi_instance.hspi,
                (uint8_t*)tx_data.data(),
                (uint8_t*)rx_data.data(),
                size / frame_size
            );
            return check_error_code(error_code);
        }

        /**
         * @brief Sends a span and receives a trivially copyable type over SPI using DMA, uses an
         * optional operation flag to signal completion.
         */
        template <typename E, size_t S, typename T>
        bool transceive_DMA(span<E, S> tx_data, T& rx_data, volatile bool* operation_flag = nullptr)
            requires std::is_trivially_copyable_v<T>
        {
            spi_instance.operation_flag = operation_flag;
            auto size = std::min(tx_data.size_bytes(), sizeof(T));
            if (size % frame_size != 0) {
                ErrorHandler(
                    "SPI transaction size (%d) not aligned to frame size (%d)",
                    size,
                    frame_size
                );
                return false;
            }
            auto error_code = HAL_SPI_TransmitReceive_DMA(
                &spi_instance.hspi,
                (uint8_t*)tx_data.data(),
                reinterpret_cast<uint8_t*>(&rx_data),
                size / frame_size
            );
            return check_error_code(error_code);
        }

        /**
         * @brief Sends a trivially copyable type and receives a span over SPI using DMA, uses an
         * optional operation flag to signal completion.
         */
        template <typename T, typename E, size_t S>
        bool transceive_DMA(
            const T& tx_data,
            span<E, S> rx_data,
            volatile bool* operation_flag = nullptr
        )
            requires std::is_trivially_copyable_v<T>
        {
            spi_instance.operation_flag = operation_flag;
            auto size = std::min(sizeof(T), rx_data.size_bytes());
            if (size % frame_size != 0) {
                ErrorHandler(
                    "SPI transaction size (%d) not aligned to frame size (%d)",
                    size,
                    frame_size
                );
                return false;
            }
            auto error_code = HAL_SPI_TransmitReceive_DMA(
                &spi_instance.hspi,
                reinterpret_cast<const uint8_t*>(&tx_data),
                (uint8_t*)rx_data.data(),
                size / frame_size
            );
            return check_error_code(error_code);
        }

        /**
         * @brief Sends and receives a trivially copyable data type over SPI using DMA, uses an
         * optional operation flag to signal completion.
         */
        template <typename T1, typename T2>
        bool transceive_DMA(const T1& tx_data, T2& rx_data, volatile bool* operation_flag = nullptr)
            requires(std::is_trivially_copyable_v<T1> && std::is_trivially_copyable_v<T2>)
        {
            spi_instance.operation_flag = operation_flag;
            auto size = std::min(sizeof(T1), sizeof(T2));
            if (size % frame_size != 0) {
                ErrorHandler(
                    "SPI transaction size (%d) not aligned to frame size (%d)",
                    size,
                    frame_size
                );
                return false;
            }
            auto error_code = HAL_SPI_TransmitReceive_DMA(
                &spi_instance.hspi,
                reinterpret_cast<const uint8_t*>(&tx_data),
                reinterpret_cast<uint8_t*>(&rx_data),
                size / frame_size
            );
            return check_error_code(error_code);
        }

    private:
        Instance& spi_instance;
        bool check_error_code(HAL_StatusTypeDef error_code) {
            if (error_code == HAL_OK) {
                return true;
            } else if (error_code == HAL_BUSY) {
                return false;
            } else {
                ErrorHandler("SPI transmit error: %u", static_cast<uint8_t>(error_code));
                return false;
            }
        }
    };

    /**
     * @brief SPI Wrapper for Slave mode operations. Doesn't allow for blocking operations.
     */
    template <auto& device_request> struct SPIWrapper<device_request, false> {
        static constexpr uint32_t data_bits =
            static_cast<uint32_t>(device_request.config.data_size);
        static constexpr uint32_t frame_size = (data_bits <= 8) ? 1 : ((data_bits <= 16) ? 2 : 4);

        SPIWrapper(Instance& instance) : spi_instance{instance} {}

        void set_software_nss(bool selected)
            requires(device_request.config.nss_mode == SPIConfigTypes::NSSMode::SOFTWARE)
        {
            if (selected) {
                CLEAR_BIT(spi_instance.instance->CR1, SPI_CR1_SSI);
            } else {
                SET_BIT(spi_instance.instance->CR1, SPI_CR1_SSI);
            }
        }

        /**
         * @brief Aborts any ongoing SPI operation and recovers the peripheral.
         */
        void abort_and_recover() { spi_instance.recover(); }

        /**
         * @brief Gets the error count for this SPI instance.
         */
        uint32_t get_error_count() const { return spi_instance.error_count; }

        /**
         * @brief Indicates whether the last operation was aborted.
         */
        bool was_aborted() const { return spi_instance.was_aborted; }

        /**
         * @brief Clears the aborted state flag for this SPI instance.
         */
        void clear_abort_flag() { spi_instance.was_aborted = false; }

        /**
         * @brief Listens for data over SPI using DMA, uses an optional operation flag to signal
         * completion.
         */
        template <typename E, size_t S>
        bool listen(span<E, S> data, volatile bool* operation_flag = nullptr) {
            spi_instance.operation_flag = operation_flag;
            if (data.size_bytes() % frame_size != 0) {
                ErrorHandler(
                    "SPI data size (%d) not aligned to frame size (%d)",
                    data.size_bytes(),
                    frame_size
                );
                return false;
            }
            auto error_code = HAL_SPI_Receive_DMA(
                &spi_instance.hspi,
                (uint8_t*)data.data(),
                data.size_bytes() / frame_size
            );
            return check_error_code(error_code);
        }

        /**
         * @brief Listens for trivially copyable data type over SPI using DMA, uses an optional
         * operation flag to signal completion.
         */
        template <typename T>
        bool listen(T& data, volatile bool* operation_flag = nullptr)
            requires std::is_trivially_copyable_v<T>
        {
            spi_instance.operation_flag = operation_flag;
            if (sizeof(T) % frame_size != 0) {
                ErrorHandler(
                    "SPI data size (%d) not aligned to frame size (%d)",
                    sizeof(T),
                    frame_size
                );
                return false;
            }
            auto error_code = HAL_SPI_Receive_DMA(
                &spi_instance.hspi,
                reinterpret_cast<uint8_t*>(&data),
                sizeof(T) / frame_size
            );
            return check_error_code(error_code);
        }

        /**
         * @brief Arms the SPI to send data over DMA when requested, uses an optional operation flag
         * to signal completion.
         */
        template <typename E, size_t S>
        bool arm(span<E, S> tx_data, volatile bool* operation_flag = nullptr) {
            spi_instance.operation_flag = operation_flag;
            if (tx_data.size_bytes() % frame_size != 0) {
                ErrorHandler(
                    "SPI data size (%d) not aligned to frame size (%d)",
                    tx_data.size_bytes(),
                    frame_size
                );
                return false;
            }
            auto error_code = HAL_SPI_Transmit_DMA(
                &spi_instance.hspi,
                (uint8_t*)tx_data.data(),
                tx_data.size_bytes() / frame_size
            );
            return check_error_code(error_code);
        }

        /**
         * @brief Arms the SPI to send a trivially copyable data type over DMA when requested, uses
         * an optional operation flag to signal completion.
         */
        template <typename T>
        bool arm(const T& data, volatile bool* operation_flag = nullptr)
            requires std::is_trivially_copyable_v<T>
        {
            spi_instance.operation_flag = operation_flag;
            if (sizeof(T) % frame_size != 0) {
                ErrorHandler(
                    "SPI data size (%d) not aligned to frame size (%d)",
                    sizeof(T),
                    frame_size
                );
                return false;
            }
            auto error_code = HAL_SPI_Transmit_DMA(
                &spi_instance.hspi,
                reinterpret_cast<const uint8_t*>(&data),
                sizeof(T) / frame_size
            );
            return check_error_code(error_code);
        }

        /**
         * @brief Sends and receives data over SPI using DMA, uses an optional operation flag to
         * signal completion.
         */
        template <typename E1, size_t S1, typename E2, size_t S2>
        bool transceive(
            span<E1, S1> tx_data,
            span<E2, S2> rx_data,
            volatile bool* operation_flag = nullptr
        ) {
            spi_instance.operation_flag = operation_flag;
            auto size = std::min(tx_data.size_bytes(), rx_data.size_bytes());
            if (size % frame_size != 0) {
                ErrorHandler(
                    "SPI transaction size (%d) not aligned to frame size (%d)",
                    size,
                    frame_size
                );
                return false;
            }
            auto error_code = HAL_SPI_TransmitReceive_DMA(
                &spi_instance.hspi,
                (uint8_t*)tx_data.data(),
                (uint8_t*)rx_data.data(),
                size / frame_size
            );
            return check_error_code(error_code);
        }

        /**
         * @brief Sends a span and receives a trivially copyable type over SPI using DMA, uses an
         * optional operation flag to signal completion.
         */
        template <typename E, size_t S, typename T>
        bool transceive(span<E, S> tx_data, T& rx_data, volatile bool* operation_flag = nullptr)
            requires std::is_trivially_copyable_v<T>
        {
            spi_instance.operation_flag = operation_flag;
            auto size = std::min(tx_data.size_bytes(), sizeof(T));
            if (size % frame_size != 0) {
                ErrorHandler(
                    "SPI transaction size (%d) not aligned to frame size (%d)",
                    size,
                    frame_size
                );
                return false;
            }
            auto error_code = HAL_SPI_TransmitReceive_DMA(
                &spi_instance.hspi,
                (uint8_t*)tx_data.data(),
                reinterpret_cast<uint8_t*>(&rx_data),
                size / frame_size
            );
            return check_error_code(error_code);
        }

        /**
         * @brief Sends a trivially copyable type and receives a span over SPI using DMA, uses an
         * optional operation flag to signal completion.
         */
        template <typename T, typename E, size_t S>
        bool
        transceive(const T& tx_data, span<E, S> rx_data, volatile bool* operation_flag = nullptr)
            requires std::is_trivially_copyable_v<T>
        {
            spi_instance.operation_flag = operation_flag;
            auto size = std::min(sizeof(T), rx_data.size_bytes());
            if (size % frame_size != 0) {
                ErrorHandler(
                    "SPI transaction size (%d) not aligned to frame size (%d)",
                    size,
                    frame_size
                );
                return false;
            }
            auto error_code = HAL_SPI_TransmitReceive_DMA(
                &spi_instance.hspi,
                reinterpret_cast<const uint8_t*>(&tx_data),
                (uint8_t*)rx_data.data(),
                size / frame_size
            );
            return check_error_code(error_code);
        }

        /**
         * @brief Sends and receives a trivially copyable data type over SPI using DMA, uses an
         * optional operation flag to signal completion.
         */
        template <typename T1, typename T2>
        bool transceive(const T1& tx_data, T2& rx_data, volatile bool* operation_flag = nullptr)
            requires(std::is_trivially_copyable_v<T1> && std::is_trivially_copyable_v<T2>)
        {
            spi_instance.operation_flag = operation_flag;
            auto size = std::min(sizeof(T1), sizeof(T2));
            if (size % frame_size != 0) {
                ErrorHandler(
                    "SPI transaction size (%d) not aligned to frame size (%d)",
                    size,
                    frame_size
                );
                return false;
            }
            auto error_code = HAL_SPI_TransmitReceive_DMA(
                &spi_instance.hspi,
                reinterpret_cast<const uint8_t*>(&tx_data),
                reinterpret_cast<uint8_t*>(&rx_data),
                size / frame_size
            );
            return check_error_code(error_code);
        }

    private:
        Instance& spi_instance;
        bool check_error_code(HAL_StatusTypeDef error_code) {
            if (error_code == HAL_OK) {
                return true;
            } else if (error_code == HAL_BUSY) {
                return false;
            } else {
                ErrorHandler("SPI transmit error: %u", static_cast<uint8_t>(error_code));
                return false;
            }
        }
    };

    /**
     * =========================================
     *          Internal working things
     * =========================================
     */
    template <size_t N> static consteval array<Config, N> build(span<const Entry> entries) {
        array<Config, N> cfgs{};

        if (N == 0) {
            return cfgs;
        }

        bool used_peripherals[6] = {false};

        for (std::size_t i = 0; i < N; ++i) {
            cfgs[i].peripheral = entries[i].peripheral;
            cfgs[i].mode = entries[i].mode;
            cfgs[i].sck_gpio_idx = entries[i].sck_gpio_idx;
            cfgs[i].miso_gpio_idx = entries[i].miso_gpio_idx;
            cfgs[i].mosi_gpio_idx = entries[i].mosi_gpio_idx;
            cfgs[i].nss_gpio_idx = entries[i].nss_gpio_idx;
            cfgs[i].dma_rx_idx = entries[i].dma_rx_idx;
            cfgs[i].dma_tx_idx = entries[i].dma_tx_idx;
            cfgs[i].max_baudrate = entries[i].max_baudrate;
            cfgs[i].config = entries[i].config;

            auto peripheral = entries[i].peripheral;

            if (peripheral == SPIPeripheral::spi1) {
                if (used_peripherals[0]) {
                    compile_error("SPI1 peripheral already used");
                }
                used_peripherals[0] = true;
            } else if (peripheral == SPIPeripheral::spi2) {
                if (used_peripherals[1]) {
                    compile_error("SPI2 peripheral already used");
                }
                used_peripherals[1] = true;
            } else if (peripheral == SPIPeripheral::spi3) {
                if (used_peripherals[2]) {
                    compile_error("SPI3 peripheral already used");
                }
                used_peripherals[2] = true;
            } else if (peripheral == SPIPeripheral::spi4) {
                if (used_peripherals[3]) {
                    compile_error("SPI4 peripheral already used");
                }
                used_peripherals[3] = true;
            } else if (peripheral == SPIPeripheral::spi5) {
                if (used_peripherals[4]) {
                    compile_error("SPI5 peripheral already used");
                }
                used_peripherals[4] = true;
            } else if (peripheral == SPIPeripheral::spi6) {
                if (used_peripherals[5]) {
                    compile_error("SPI6 peripheral already used");
                }
                used_peripherals[5] = true;
            }
        }

        return cfgs;
    }

    template <std::size_t N> struct Init {
        static inline std::array<Instance, N> instances{};

        static void init(
            std::span<const Config, N> cfgs,
            std::span<GPIODomain::Instance> gpio_instances,
            std::span<DMA_Domain::Instance> dma_peripherals
        ) {
            for (std::size_t i = 0; i < N; ++i) {
                const auto& e = cfgs[i];

                SPIPeripheral peripheral = e.peripheral;
                instances[i].instance = reinterpret_cast<SPI_TypeDef*>(e.peripheral);

                // Configure clock and store handle
                RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
                uint8_t spi_number = 0;
                if (peripheral == SPIPeripheral::spi1) {
                    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPI1;
                    PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL;
                    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
                        ErrorHandler("Unable to configure SPI1 clock");
                    }
                    __HAL_RCC_SPI1_CLK_ENABLE();
                    spi_number = 1;
                } else if (peripheral == SPIPeripheral::spi2) {
                    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPI2;
                    PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL;
                    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
                        ErrorHandler("Unable to configure SPI2 clock");
                    }
                    __HAL_RCC_SPI2_CLK_ENABLE();
                    spi_number = 2;
                } else if (peripheral == SPIPeripheral::spi3) {
                    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPI3;
                    PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL;
                    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
                        ErrorHandler("Unable to configure SPI3 clock");
                    }
                    __HAL_RCC_SPI3_CLK_ENABLE();
                    spi_number = 3;
                } else if (peripheral == SPIPeripheral::spi4) {
                    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPI4;
                    PeriphClkInitStruct.Spi45ClockSelection = RCC_SPI45CLKSOURCE_PLL2;
                    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
                        ErrorHandler("Unable to configure SPI4 clock");
                    }
                    __HAL_RCC_SPI4_CLK_ENABLE();
                    spi_number = 4;
                } else if (peripheral == SPIPeripheral::spi5) {
                    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPI5;
                    PeriphClkInitStruct.Spi45ClockSelection = RCC_SPI45CLKSOURCE_PLL2;
                    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
                        ErrorHandler("Unable to configure SPI5 clock");
                    }
                    __HAL_RCC_SPI5_CLK_ENABLE();
                    spi_number = 5;
                } else if (peripheral == SPIPeripheral::spi6) {
                    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPI6;
                    PeriphClkInitStruct.Spi6ClockSelection = RCC_SPI6CLKSOURCE_PLL2;
                    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
                        ErrorHandler("Unable to configure SPI6 clock");
                    }
                    __HAL_RCC_SPI6_CLK_ENABLE();
                    spi_number = 6;
                }

                auto& hspi = instances[i].hspi;
                hspi.Instance = instances[i].instance;

                auto& dma_rx = dma_peripherals[e.dma_rx_idx];
                auto& dma_tx = dma_peripherals[e.dma_tx_idx];

                // DMA handles are already configured and initialized by DMA_Domain
                hspi.hdmarx = &dma_rx.dma;
                hspi.hdmatx = &dma_tx.dma;

                // Link back from DMA to SPI (required by HAL)
                dma_rx.dma.Parent = &hspi;
                dma_tx.dma.Parent = &hspi;

                if (e.config.data_size > DataSize::SIZE_16BIT) {
                    dma_rx.dma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
                    dma_rx.dma.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
                    dma_tx.dma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
                    dma_tx.dma.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
                } else if (e.config.data_size > DataSize::SIZE_8BIT) {
                    dma_rx.dma.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
                    dma_rx.dma.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
                    dma_tx.dma.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
                    dma_tx.dma.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
                }

                HAL_DMA_Init(hspi.hdmarx);
                HAL_DMA_Init(hspi.hdmatx);

                auto& init = hspi.Init;
                if (e.mode == SPIMode::MASTER) {
                    init.Mode = SPI_MODE_MASTER;
                    // Baudrate prescaler calculation
                    uint32_t pclk_freq;
                    if (peripheral == SPIPeripheral::spi1 || peripheral == SPIPeripheral::spi2 ||
                        peripheral == SPIPeripheral::spi3) {
                        pclk_freq = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SPI123);
                    } else if (peripheral == SPIPeripheral::spi4 || peripheral == SPIPeripheral::spi5) {
                        pclk_freq = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SPI45);
                    } else {
                        pclk_freq = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SPI6);
                    }
                    init.BaudRatePrescaler = calculate_prescaler(pclk_freq, e.max_baudrate);
                } else {
                    init.Mode = SPI_MODE_SLAVE;
                }

                init.NSS = SPIConfigTypes::translate_nss_mode(
                    e.config.nss_mode,
                    e.mode == SPIMode::MASTER
                );
                init.Direction = SPIConfigTypes::translate_direction(e.config.direction);
                init.DataSize = SPIConfigTypes::translate_data_size(e.config.data_size);
                init.CLKPolarity = SPIConfigTypes::translate_clock_polarity(e.config.polarity);
                init.CLKPhase = SPIConfigTypes::translate_clock_phase(e.config.phase);
                init.FirstBit = SPIConfigTypes::translate_bit_order(e.config.bit_order);
                init.TIMode = SPIConfigTypes::translate_ti_mode(e.config.ti_mode);
                init.CRCCalculation =
                    SPIConfigTypes::translate_crc_calculation(e.config.crc_calculation);
                if (e.config.crc_calculation) {
                    init.CRCPolynomial = e.config.crc_polynomial;
                    init.CRCLength = SPIConfigTypes::translate_crc_length(e.config.crc_length);
                }
                init.NSSPMode = SPIConfigTypes::translate_nss_pulse(e.config.nss_pulse);
                init.NSSPolarity = SPIConfigTypes::translate_nss_polarity(e.config.nss_polarity);
                init.FifoThreshold =
                    SPIConfigTypes::translate_fifo_threshold(e.config.fifo_threshold);
                init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
                init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
                init.MasterSSIdleness =
                    SPIConfigTypes::translate_ss_idleness(e.config.master_ss_idleness);
                init.MasterInterDataIdleness =
                    SPIConfigTypes::translate_interdata_idleness(e.config.master_interdata_idleness
                    );
                init.MasterReceiverAutoSusp =
                    SPIConfigTypes::translate_rx_autosusp(e.config.master_rx_autosusp);
                init.MasterKeepIOState =
                    SPIConfigTypes::translate_keep_io_state(e.config.keep_io_state);
                init.IOSwap = SPIConfigTypes::translate_io_swap(e.config.io_swap);

                if (HAL_SPI_Init(&hspi) != HAL_OK) {
                    ErrorHandler("Unable to init SPI%u", spi_number);
                    return;
                }

                // Enable NVIC
                if (peripheral == SPIPeripheral::spi1) {
                    HAL_NVIC_SetPriority(SPI1_IRQn, 1, 0);
                    HAL_NVIC_EnableIRQ(SPI1_IRQn);
                } else if (peripheral == SPIPeripheral::spi2) {
                    HAL_NVIC_SetPriority(SPI2_IRQn, 1, 0);
                    HAL_NVIC_EnableIRQ(SPI2_IRQn);
                } else if (peripheral == SPIPeripheral::spi3) {
                    HAL_NVIC_SetPriority(SPI3_IRQn, 1, 0);
                    HAL_NVIC_EnableIRQ(SPI3_IRQn);
                } else if (peripheral == SPIPeripheral::spi4) {
                    HAL_NVIC_SetPriority(SPI4_IRQn, 1, 0);
                    HAL_NVIC_EnableIRQ(SPI4_IRQn);
                } else if (peripheral == SPIPeripheral::spi5) {
                    HAL_NVIC_SetPriority(SPI5_IRQn, 1, 0);
                    HAL_NVIC_EnableIRQ(SPI5_IRQn);
                } else if (peripheral == SPIPeripheral::spi6) {
                    HAL_NVIC_SetPriority(SPI6_IRQn, 1, 0);
                    HAL_NVIC_EnableIRQ(SPI6_IRQn);
                }

                // Map instance pointer
                if (peripheral == SPIPeripheral::spi1) {
                    spi_instances[0] = &instances[i];
                } else if (peripheral == SPIPeripheral::spi2) {
                    spi_instances[1] = &instances[i];
                } else if (peripheral == SPIPeripheral::spi3) {
                    spi_instances[2] = &instances[i];
                } else if (peripheral == SPIPeripheral::spi4) {
                    spi_instances[3] = &instances[i];
                } else if (peripheral == SPIPeripheral::spi5) {
                    spi_instances[4] = &instances[i];
                } else if (peripheral == SPIPeripheral::spi6) {
                    spi_instances[5] = &instances[i];
                }
            }
        }
    };
};

} // namespace ST_LIB

#endif // SPI2_HPP
