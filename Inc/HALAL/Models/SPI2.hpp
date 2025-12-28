/*
 * SPI2.hpp
 *
 *  Created on: 27 dec. 2025
 *      Author: Boris
 */

#ifndef SPI2_HPP
#define SPI2_HPP

#include "C++Utilities/CppImports.hpp"
#include "HALAL/Models/GPIO.hpp"
#include "ErrorHandler/ErrorHandler.hpp"

using ST_LIB::GPIODomain;

extern ST_LIB::SPIDomain::Instance* spi_instances[ST_LIB::SPIDomain::max_instances];

namespace ST_LIB {

struct SPIDomain {

/**
 * =========================================
 *          Internal working things
 * =========================================
 */

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
    }

    consteval GPIODomain::AlternateFunction get_af(GPIODomain::Pin &pin, SPIPeripheral peripheral) {
        if (peripheral == SPIPeripheral::spi2) {
            if (pin == PB4) return GPIODomain::AlternateFunction::AF7;
        }
        if (peripheral == SPIPeripheral::spi3) {
            if (pin == PA4) return GPIODomain::AlternateFunction::AF6;
            if (pin == PA15) return GPIODomain::AlternateFunction::AF6;
            if (pin == PB2) return GPIODomain::AlternateFunction::AF7;
            if (pin == PB3) return GPIODomain::AlternateFunction::AF6;
            if (pin == PB4) return GPIODomain::AlternateFunction::AF6;
            if (pin == PB5) return GPIODomain::AlternateFunction::AF7;
            if (pin == PC10) return GPIODomain::AlternateFunction::AF6;
            if (pin == PC11) return GPIODomain::AlternateFunction::AF6;
            if (pin == PC12) return GPIODomain::AlternateFunction::AF6;
        }
        if (peripheral == SPIPeripheral::spi6) {
            if (pin == PA4) return GPIODomain::AlternateFunction::AF8;
            if (pin == PA5) return GPIODomain::AlternateFunction::AF8;
            if (pin == PA6) return GPIODomain::AlternateFunction::AF8;
            if (pin == PA7) return GPIODomain::AlternateFunction::AF8;
            if (pin == PA15) return GPIODomain::AlternateFunction::AF7;
            if (pin == PB3) return GPIODomain::AlternateFunction::AF8;
            if (pin == PB4) return GPIODomain::AlternateFunction::AF8;
            if (pin == PB5) return GPIODomain::AlternateFunction::AF8;
        }

        return GPIODomain::AlternateFunction::AF5; // Default AF for everything else
    }

    static constexpr uint32_t get_prescaler_flag(uint32_t prescaler) {
        switch (prescaler) {
            case 2: return SPI_BAUDRATEPRESCALER_2;
            case 4: return SPI_BAUDRATEPRESCALER_4;
            case 8: return SPI_BAUDRATEPRESCALER_8;
            case 16: return SPI_BAUDRATEPRESCALER_16;
            case 32: return SPI_BAUDRATEPRESCALER_32;
            case 64: return SPI_BAUDRATEPRESCALER_64;
            case 128: return SPI_BAUDRATEPRESCALER_128;
            case 256: return SPI_BAUDRATEPRESCALER_256;
            default:
                if consteval {
                    compile_error("Invalid prescaler value");
                } else {
                    ErrorHandler("Invalid prescaler value");
                    return SPI_BAUDRATEPRESCALER_256;
                }
        }
    }

    static uint32_t calculate_prescaler(uint32_t src_freq, uint32_t max_baud) {
        uint32_t prescaler = 2; // Smallest prescaler available

        while ((src_freq / prescaler) > max_baud) {
            prescaler *= 2; // Prescaler doubles each step (it must be a power of 2)

            if (prescaler > 256) {
                ErrorHandler("Cannot achieve desired baudrate, speed is too low");
            }
        }
        
        return get_prescaler_flag(prescaler);
    }

    static constexpr std::size_t max_instances{6};

    struct Entry {
        SPIPeripheral peripheral;
        SPIMode mode;

        std::size_t sck_gpio_idx;
        std::size_t miso_gpio_idx;
        std::size_t mosi_gpio_idx;
        std::size_t nss_gpio_idx;

        uint32_t max_baudrate; // Will set the baudrate as fast as possible under this value

        // DMA here, maybe? Depends on new DMA implementation
    };

    struct Config {
        SPIPeripheral peripheral;
        SPIMode mode;

        std::size_t sck_gpio_idx;
        std::size_t miso_gpio_idx;
        std::size_t mosi_gpio_idx;
        std::size_t nss_gpio_idx;

        uint32_t max_baudrate; // Will set the baudrate as fast as possible under this value

        // DMA here, maybe? Depends on new DMA implementation
    };


/**
 * =========================================
 *              Request Object
 * =========================================
 */
    struct Device {
        using domain = SPIDomain;

        SPIPeripheral peripheral;
        SPIMode mode;

        GPIODomain::GPIO sck_gpio;
        GPIODomain::GPIO miso_gpio;
        GPIODomain::GPIO mosi_gpio;
        GPIODomain::GPIO nss_gpio;

        uint32_t max_baudrate; // Will set the baudrate as fast as possible under this value

        consteval Device(SPIMode mode, SPIPeripheral peripheral, uint32_t max_baudrate,
                        GPIODomain::Pin sck_pin, GPIODomain::Pin miso_pin, 
                        GPIODomain::Pin mosi_pin, GPIODomain::Pin nss_pin)
                        : peripheral{peripheral}, mode{mode}, max_baudrate{max_baudrate} {

            switch (peripheral) {
            case SPIPeripheral::spi1:
                if (sck_pin != PB3 &&
                    sck_pin != PG11 &&
                    sck_pin != PA5) {
                    compile_error("Invalid SCK pin for SPI1");
                }
                if (miso_pin != PB4 &&
                    miso_pin != PG9 &&
                    miso_pin != PA6) {
                    compile_error("Invalid MISO pin for SPI1");
                }
                if (mosi_pin != PB5 &&
                    mosi_pin != PD7 &&
                    mosi_pin != PA7) {
                    compile_error("Invalid MOSI pin for SPI1");
                }
                if (nss_pin != PG10 &&
                    nss_pin != PA15 &&
                    nss_pin != PA4) {
                    compile_error("Invalid NSS pin for SPI1");
                }
                break;

            case SPIPeripheral::spi2:
                if (sck_pin != PD3 &&
                    sck_pin != PA12 &&
                    sck_pin != PA9 &&
                    sck_pin != PB13 &&
                    sck_pin != PB10) {
                    compile_error("Invalid SCK pin for SPI2");
                }
                if (miso_pin != PC2 &&
                    miso_pin != PB14) {
                    compile_error("Invalid MISO pin for SPI2");
                }
                if (mosi_pin != PC3 &&
                    mosi_pin != PC1 &&
                    mosi_pin != PB15) {
                    compile_error("Invalid MOSI pin for SPI2");
                }
                if (nss_pin != PB9 &&
                    nss_pin != PB4 &&
                    nss_pin != PA11 &&
                    nss_pin != PB12) {
                    compile_error("Invalid NSS pin for SPI2");
                }
                break;
            
            case SPIPeripheral::spi3:
                if (sck_pin != PB3 &&
                    sck_pin != PC10) {
                    compile_error("Invalid SCK pin for SPI3");
                }    
                if (miso_pin != PB4 &&
                    miso_pin != PC11) {
                    compile_error("Invalid MISO pin for SPI3");
                }
                if (mosi_pin != PB5 &&
                    mosi_pin != PD6 &&
                    mosi_pin != PC12 &&
                    mosi_pin != PB2) {
                    compile_error("Invalid MOSI pin for SPI3");
                }
                if (nss_pin != PA15 &&
                    nss_pin != PA4) {
                    compile_error("Invalid NSS pin for SPI3");
                }
                break;

            case SPIPeripheral::spi4:
                if (sck_pin != PE2 &&
                    sck_pin != PE12) {
                    compile_error("Invalid SCK pin for SPI4");
                }
                if (miso_pin != PE5 &&
                    miso_pin != PE13) {
                    compile_error("Invalid MISO pin for SPI4");
                }
                if (mosi_pin != PE6 &&
                    mosi_pin != PE14) {
                    compile_error("Invalid MOSI pin for SPI4");
                }
                if (nss_pin != PE4 &&
                    nss_pin != PE11) {
                    compile_error("Invalid NSS pin for SPI4");
                }
                break;

            case SPIPeripheral::spi5:
                if (sck_pin != PF7) {
                    compile_error("Invalid SCK pin for SPI5");
                }
                if (miso_pin != PF8) {
                    compile_error("Invalid MISO pin for SPI5");
                }
                if (mosi_pin != PF9 &&
                    mosi_pin != PF11) {
                    compile_error("Invalid MOSI pin for SPI5");
                }
                if (nss_pin != PF6) {
                    compile_error("Invalid NSS pin for SPI5");
                }
                break;
            
            case SPIPeripheral::spi6:
                if (sck_pin != PB3 &&
                    sck_pin != PG13 &&
                    sck_pin != PC10 && 
                    sck_pin != PA7) {
                    compile_error("Invalid SCK pin for SPI6");
                }
                if (miso_pin != PB4 &&
                    miso_pin != PG12 &&
                    miso_pin != PA6) {
                    compile_error("Invalid MISO pin for SPI6");
                }
                if (mosi_pin != PB5 &&
                    mosi_pin != PG14 &&
                    mosi_pin != PA7) {
                    compile_error("Invalid MOSI pin for SPI6");
                }
                if (nss_pin != PA0 &&
                    nss_pin != PA15 &&
                    nss_pin != PG8 &&
                    nss_pin != PA4) {
                    compile_error("Invalid NSS pin for SPI6");
                }
                break;

            default:
                compile_error("Invalid SPI peripheral specified in SPIDomain::Device");
            }

            sck_gpio = GPIODomain::GPIO{sck_pin, GPIODomain::OperationMode::ALT_PP, GPIODomain::Pull::None, GPIODomain::Speed::VeryHigh, get_af(sck_pin, peripheral)};
            miso_gpio = GPIODomain::GPIO{miso_pin, GPIODomain::OperationMode::ALT_PP, GPIODomain::Pull::None, GPIODomain::Speed::VeryHigh, get_af(miso_pin, peripheral)};
            mosi_gpio = GPIODomain::GPIO{mosi_pin, GPIODomain::OperationMode::ALT_PP, GPIODomain::Pull::None, GPIODomain::Speed::VeryHigh, get_af(mosi_pin, peripheral)};
            nss_gpio = GPIODomain::GPIO{nss_pin, GPIODomain::OperationMode::ALT_PP, GPIODomain::Pull::None, GPIODomain::Speed::VeryHigh, get_af(nss_pin, peripheral)};
        }

        template <class Ctx>
        consteval void inscribe(Ctx &ctx) const {
            Entry e{
                .peripheral = peripheral,
                .mode = mode,
                .sck_gpio_idx = ctx.template add<GPIODomain>(sck_gpio.e),
                .miso_gpio_idx = ctx.template add<GPIODomain>(miso_gpio.e),
                .mosi_gpio_idx = ctx.template add<GPIODomain>(mosi_gpio.e),
                .nss_gpio_idx = ctx.template add<GPIODomain>(nss_gpio.e),
            };

            ctx.template add<SPIDomain>(e);
        }
    };


/**
 * =========================================
 *        Instance (state holder)
 * =========================================
 */
    struct Instance {
        template <std::size_t N> friend struct Init;
        template <auto &device_request> friend struct SPIWrapper;
        friend void ::SPI1_IRQHandler(void);
        friend void ::SPI2_IRQHandler(void);
        friend void ::SPI3_IRQHandler(void);
        friend void ::SPI4_IRQHandler(void);
        friend void ::SPI5_IRQHandler(void);
        friend void ::SPI6_IRQHandler(void);

       private:
        SPI_HandleTypeDef hspi;
        SPI_TypeDef* instance;

        volatile bool* operation_flag;
    };


/**
 * =========================================
 *          Wrapper, public API
 * =========================================
 */

    /**
     * @brief SPI Wrapper for Master mode operations.
     */
    template <auto &device_request> requires(device_request.mode == SPIMode::MASTER)
    struct SPIWrapper {
        SPIWrapper(Instance &instance) : spi_instance{instance} {}

        /**
         * @brief Sends data over SPI in blocking mode.
         */
        bool send(span<const uint8_t> data) {
            auto error_code = HAL_SPI_Transmit(&spi_instance.hspi, data.data(), data.size(), 10);
            return check_error_code(error_code);
        }
        
        /**
         * @brief Receives data over SPI in blocking mode.
         */
        bool receive(span<uint8_t> data) {
            auto error_code = HAL_SPI_Receive(&spi_instance.hspi, data.data(), data.size(), 10);
            return check_error_code(error_code);
        }

        /**
         * @brief Sends and receives data over SPI in blocking mode.
         */
        bool transceive(span<const uint8_t> tx_data, span<uint8_t> rx_data) {
            auto error_code = HAL_SPI_TransmitReceive(&spi_instance.hspi, tx_data.data(), rx_data.data(), tx_data.size(), 10);
            return check_error_code(error_code);
        }


        /**
         * @brief Sends data over SPI using DMA, uses an optional operation flag to signal completion.
         */
        bool send_DMA(span<const uint8_t> data, volatile bool* operation_flag = nullptr) {
            spi_instance.operation_flag = operation_flag;
            auto error_code = HAL_SPI_Transmit_DMA(&spi_instance.hspi, data.data(), data.size());
            return check_error_code(error_code);
        }

        /**
         * @brief Receives data over SPI using DMA, uses an optional operation flag to signal completion.
         */
        bool receive_DMA(span<uint8_t> data, volatile bool* operation_flag = nullptr) {
            spi_instance.operation_flag = operation_flag;
            auto error_code = HAL_SPI_Receive_DMA(&spi_instance.hspi, data.data(), data.size());
            return check_error_code(error_code);
        }

        /**
         * @brief Sends and receives data over SPI using DMA, uses an optional operation flag to signal completion.
         */
        bool transceive_DMA(span<const uint8_t> tx_data, span<uint8_t> rx_data, volatile bool* operation_flag = nullptr) {
            spi_instance.operation_flag = operation_flag;
            auto size = std::min(tx_data.size(), rx_data.size());
            auto error_code = HAL_SPI_TransmitReceive_DMA(&spi_instance.hspi, tx_data.data(), rx_data.data(), size);
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
    template <auto &device_request> requires(device_request.mode == SPIMode::SLAVE)
    struct SPIWrapper {
        SPIWrapper(Instance &instance) : spi_instance{instance} {}

        /**
         * @brief Listens for data over SPI using DMA, uses an optional operation flag to signal completion.
         */
        bool listen(span<uint8_t> data, volatile bool* operation_flag = nullptr) {
            spi_instance.operation_flag = operation_flag;
            auto error_code = HAL_SPI_Receive_DMA(&spi_instance.hspi, data.data(), data.size());
            return check_error_code(error_code);
        }

        /**
         * @brief Arms the SPI to send data over DMA when requested, uses an optional operation flag to signal completion.
         */
        bool arm(span<const uint8_t> tx_data, volatile bool* operation_flag = nullptr) {
            spi_instance.operation_flag = operation_flag;
            auto error_code = HAL_SPI_Transmit_DMA(&spi_instance.hspi, tx_data.data(), tx_data.size());
            return check_error_code(error_code);
        }

        /**
         * @brief Sends and receives data over SPI using DMA, uses an optional operation flag to signal completion.
         */
        bool transceive(span<const uint8_t> tx_data, span<uint8_t> rx_data, volatile bool* operation_flag = nullptr) {
            spi_instance.operation_flag = operation_flag;
            auto size = std::min(tx_data.size(), rx_data.size());
            auto error_code = HAL_SPI_TransmitReceive_DMA(&spi_instance.hspi, tx_data.data(), rx_data.data(), size);
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
    template <size_t N>
    static consteval array<Config, N> build(span<const Entry> entries) {
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

    template <std::size_t N>
    struct Init {
        static inline std::array<Instance, N> instances{};

        static void init(std::span<const Config, N> cfgs,
                         std::span<GPIODomain::Instance> gpio_instances) {
            for (std::size_t i = 0; i < N; ++i) {
                const auto &e = cfgs[i];

                SPIPeripheral peripheral = e.peripheral;
                instances[i].instance = reinterpret_cast<SPI_TypeDef*>(e.peripheral);

                // Configure clock and store handle
                RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
                uint8_t spi_number = 0;
                if (peripheral == SPIPeripheral::spi1) {
                    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPI1;
                    PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL;
                    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
                        ErrorHanlder("Unable to configure SPI%i clock", i+1);
                    }
                    __HAL_RCC_SPI1_CLK_ENABLE();

                    spi_number = 1;
                } else if (peripheral == SPIPeripheral::spi2) {
                    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPI2;
                    PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL;
                    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
                        ErrorHanlder("Unable to configure SPI%i clock", i+1);
                    }
                    __HAL_RCC_SPI2_CLK_ENABLE();

                    spi_number = 2;
                } else if (peripheral == SPIPeripheral::spi3) {
                    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPI3;
                    PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL;
                    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
                        ErrorHanlder("Unable to configure SPI%i clock", i+1);
                    }
                    __HAL_RCC_SPI3_CLK_ENABLE();

                    spi_number = 3;
                } else if (peripheral == SPIPeripheral::spi4) {
                    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPI4;
                    PeriphClkInitStruct.Spi45ClockSelection = RCC_SPI45CLKSOURCE_PLL2;
                    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
                        ErrorHanlder("Unable to configure SPI%i clock", i+1);
                    }
                    __HAL_RCC_SPI4_CLK_ENABLE();

                    spi_number = 4;
                } else if (peripheral == SPIPeripheral::spi5) {
                    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPI5;
                    PeriphClkInitStruct.Spi45ClockSelection = RCC_SPI45CLKSOURCE_PLL2;
                    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
                        ErrorHanlder("Unable to configure SPI%i clock", i+1);
                    }
                    __HAL_RCC_SPI5_CLK_ENABLE();

                    spi_number = 5;
                } else if (peripheral == SPIPeripheral::spi6) {
                    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPI6;
                    PeriphClkInitStruct.Spi6ClockSelection = RCC_SPI6CLKSOURCE_PLL2;
                    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
                        ErrorHanlder("Unable to configure SPI%i clock", i+1);
                    }
                    __HAL_RCC_SPI6_CLK_ENABLE();

                    spi_number = 6;
                }

                spi_instances[spi_number - 1] = &instances[i];

                auto hspi = instances[i].hspi;
                hspi.Instance = instances[i].instance;

                auto init = hspi.Init;
                if (e.mode == SPIMode::MASTER) {
                    init.Mode = SPI_MODE_MASTER;
                    init.NSS = SPI_NSS_HARD_OUTPUT; // Hardware control for now, should add software later for more flexibility
                    // Baudrate prescaler calculation
                    uint32_t pclk_freq;
                    if (peripheral == SPIPeripheral::spi1 ||
                        peripheral == SPIPeripheral::spi2 ||
                        peripheral == SPIPeripheral::spi3) {
                        pclk_freq = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SPI123);
                    } else if (peripheral == SPIPeripheral::spi4 ||
                               peripheral == SPIPeripheral::spi5) {
                        pclk_freq = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SPI45);
                    } else {
                        pclk_freq = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SPI6);
                    }
                    init.BaudRatePrescaler = calculate_prescaler(pclk_freq, e.max_baudrate);

                } else {
                    init.Mode = SPI_MODE_SLAVE;
                    init.NSS = SPI_NSS_HARD_INPUT;
                }
                init.Direction = SPI_DIRECTION_2LINES;
                init.DataSize = SPI_DATASIZE_8BIT; // Works with any data size (at least for bytes)
                init.CLKPolarity = SPI_POLARITY_LOW;
                init.CLKPhase = SPI_PHASE_1EDGE;
                // Calculate BaudRatePrescaler
                init.BaudRatePrescaler = // TODO
                init.FirstBit = SPI_FIRSTBIT_MSB; // Must check if LSB first is needed for anything later
                init.TIMode = SPI_TIMODE_DISABLE; // Texas Instruments mode, like, why would we use that?
                init.CRCCalculation = SPI_CRCCALCULATION_DISABLE; // Doesn't seem that useful here, better to handle CRC manually with the CRC peripheral if needed
                init.CRCPolynomial = 0; // Nope
                init.CRCLength = SPI_CRC_LENGTH_8BIT; // Doesn't matter since CRC calculation is disabled
                init.NSSPMode = SPI_NSS_PULSE_DISABLE; // Hardcoded for now, may add a setting later
                init.NSSPolarity = SPI_NSS_POLARITY_LOW; // Standard polarity
                init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA; // 1 byte, since we're using 8 bit data size for safety, may add a setting later
                init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
                init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
                init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE; // Should check if this works or the peripheral needs some delay
                init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
                init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE; // If you are having overrun issues, then you have a problem somewhere else
                init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE; // Keeps MOSI, MISO, SCK state when not communicating, ensure no floating lines and no random noise
                init.IOSwap = SPI_IO_SWAP_DISABLE; // Should not be needed


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
            }
        }
    };
};

} // namespace ST_LIB


#endif // SPI2_HPP