/*
 * SPIConfig.hpp
 *
 *  Created on: 19 jan. 2026
 *      Author: Boris
 */

#ifndef SPICONFIG_HPP
#define SPICONFIG_HPP

#include "C++Utilities/CppImports.hpp"
#include "ErrorHandler/ErrorHandler.hpp"

namespace ST_LIB {

struct SPIConfigTypes {
    
    enum class ClockPolarity : bool {
        LOW = false,    // Clock idle state is low (CPOL=0)
        HIGH = true     // Clock idle state is high (CPOL=1)
    };

    enum class ClockPhase : bool {
        FIRST_EDGE = false,     // Data sampled on first clock edge (CPHA=0)
        SECOND_EDGE = true      // Data sampled on second clock edge (CPHA=1)
    };

    enum class BitOrder : bool {
        MSB_FIRST = false,
        LSB_FIRST = true
    };

    enum class NSSMode {
        SOFTWARE,       // Software NSS management (manual control)
        HARDWARE        // Hardware NSS management (automatic)
    };

    enum class DataSize : uint8_t {
        SIZE_4BIT = 4,
        SIZE_5BIT = 5,
        SIZE_6BIT = 6,
        SIZE_7BIT = 7,
        SIZE_8BIT = 8,
        SIZE_9BIT = 9,
        SIZE_10BIT = 10,
        SIZE_11BIT = 11,
        SIZE_12BIT = 12,
        SIZE_13BIT = 13,
        SIZE_14BIT = 14,
        SIZE_15BIT = 15,
        SIZE_16BIT = 16,
        SIZE_17BIT = 17,
        SIZE_18BIT = 18,
        SIZE_19BIT = 19,
        SIZE_20BIT = 20,
        SIZE_21BIT = 21,
        SIZE_22BIT = 22,
        SIZE_23BIT = 23,
        SIZE_24BIT = 24,
        SIZE_25BIT = 25,
        SIZE_26BIT = 26,
        SIZE_27BIT = 27,
        SIZE_28BIT = 28,
        SIZE_29BIT = 29,
        SIZE_30BIT = 30,
        SIZE_31BIT = 31,
        SIZE_32BIT = 32
    };

    enum class Direction {
    FULL_DUPLEX,        // 2-line bidirectional (most common)
    HALF_DUPLEX,        // 1-line bidirectional
    SIMPLEX_TX_ONLY,    // Transmit only
    SIMPLEX_RX_ONLY     // Receive only
    };

    enum class FIFOThreshold : uint8_t {
        THRESHOLD_01DATA = 1,
        THRESHOLD_02DATA = 2,
        THRESHOLD_03DATA = 3,
        THRESHOLD_04DATA = 4,
        THRESHOLD_05DATA = 5,
        THRESHOLD_06DATA = 6,
        THRESHOLD_07DATA = 7,
        THRESHOLD_08DATA = 8,
        THRESHOLD_09DATA = 9,
        THRESHOLD_10DATA = 10,
        THRESHOLD_11DATA = 11,
        THRESHOLD_12DATA = 12,
        THRESHOLD_13DATA = 13,
        THRESHOLD_14DATA = 14,
        THRESHOLD_15DATA = 15,
        THRESHOLD_16DATA = 16
    };

    enum class NSSPolarity {
        ACTIVE_LOW = false,
        ACTIVE_HIGH = true
    };

    enum class CRCLength : uint8_t {
        DATASIZE = 0,  // CRC length matches data size
        CRC_4BIT = 4,
        CRC_5BIT = 5,
        CRC_6BIT = 6,
        CRC_7BIT = 7,
        CRC_8BIT = 8,
        CRC_9BIT = 9,
        CRC_10BIT = 10,
        CRC_11BIT = 11,
        CRC_12BIT = 12,
        CRC_13BIT = 13,
        CRC_14BIT = 14,
        CRC_15BIT = 15,
        CRC_16BIT = 16,
        CRC_17BIT = 17,
        CRC_18BIT = 18,
        CRC_19BIT = 19,
        CRC_20BIT = 20,
        CRC_21BIT = 21,
        CRC_22BIT = 22,
        CRC_23BIT = 23,
        CRC_24BIT = 24,
        CRC_25BIT = 25,
        CRC_26BIT = 26,
        CRC_27BIT = 27,
        CRC_28BIT = 28,
        CRC_29BIT = 29,
        CRC_30BIT = 30,
        CRC_31BIT = 31,
        CRC_32BIT = 32
    };

    /**
     * @brief User-facing SPI configuration options
     */
    struct SPIConfig {
        // Core settings
        ClockPolarity polarity = ClockPolarity::LOW;
        ClockPhase phase = ClockPhase::FIRST_EDGE;
        BitOrder bit_order = BitOrder::MSB_FIRST;
        NSSMode nss_mode = NSSMode::HARDWARE;
        
        // Data format
        DataSize data_size = DataSize::SIZE_8BIT;
        Direction direction = Direction::FULL_DUPLEX;
        FIFOThreshold fifo_threshold = FIFOThreshold::THRESHOLD_01DATA;
        
        // NSS settings
        bool nss_pulse = false;                         // NSS pulse between data frames (master only)
        NSSPolarity nss_polarity = NSSPolarity::ACTIVE_LOW;
        
        // Master timing settings
        uint8_t master_ss_idleness = 0;                 // Cycles (0-15) between NSS and first data
        uint8_t master_interdata_idleness = 0;          // Cycles (0-15) between data frames
        
        // Advanced options
        bool keep_io_state = true;                      // Keep pin states when idle (prevents floating)
        bool master_rx_autosusp = false;                // Auto-suspend in master RX mode to prevent overrun
        bool io_swap = false;                           // Swap MISO/MOSI pins
        
        // Protocol options
        bool ti_mode = false;                           // Enable TI synchronous serial frame format (Microwire compatible)
        
        // CRC options (hardware CRC calculation)
        bool crc_calculation = false;                   // Enable hardware CRC calculation
        uint32_t crc_polynomial = 0x07;                 // CRC polynomial (must be odd, default for CRC-8)
        CRCLength crc_length = CRCLength::DATASIZE;     // CRC length (default: matches data size)
        
        constexpr SPIConfig() = default;
        
        constexpr SPIConfig(ClockPolarity pol, ClockPhase ph, 
                          BitOrder order = BitOrder::MSB_FIRST,
                          NSSMode nss = NSSMode::HARDWARE)
            : polarity(pol), phase(ph), bit_order(order), nss_mode(nss) {}
        
        // Compile-time validation
        constexpr void validate() const {
            // Validate CRC polynomial if CRC is enabled
            if (crc_calculation) {
                if (crc_polynomial == 0 || crc_polynomial > 0xFFFF) {
                    compile_error("CRC polynomial must be between 1 and 65535");
                }
                if ((crc_polynomial & 1) == 0) {
                    compile_error("CRC polynomial must be odd");
                }
            }
            
            // Validate timing parameters
            if (master_ss_idleness > 15) {
                compile_error("master_ss_idleness must be 0-15");
            }
            if (master_interdata_idleness > 15) {
                compile_error("master_interdata_idleness must be 0-15");
            }
        }
    };

    static constexpr uint32_t translate_nss_mode(NSSMode mode, bool is_master) {
        if (mode == NSSMode::SOFTWARE) {
            return SPI_NSS_SOFT;
        }
        // Hardware mode - depends on master/slave
        return (is_master) ? SPI_NSS_HARD_OUTPUT : SPI_NSS_HARD_INPUT;
    }

    // Translation functions from custom config to HAL constants
    static constexpr uint32_t translate_clock_polarity(ClockPolarity pol) {
        return (pol == ClockPolarity::LOW) ? SPI_POLARITY_LOW : SPI_POLARITY_HIGH;
    }

    static constexpr uint32_t translate_clock_phase(ClockPhase phase) {
        return (phase == ClockPhase::FIRST_EDGE) ? SPI_PHASE_1EDGE : SPI_PHASE_2EDGE;
    }

    static constexpr uint32_t translate_bit_order(BitOrder order) {
        return (order == BitOrder::MSB_FIRST) ? SPI_FIRSTBIT_MSB : SPI_FIRSTBIT_LSB;
    }

    static constexpr uint32_t translate_data_size(DataSize size) {
        // HAL uses SIZE-1 (e.g., 8 bits = 0x07)
        return static_cast<uint32_t>(size) - 1;
    }

    static constexpr uint32_t translate_direction(Direction dir) {
        switch (dir) {
            case Direction::FULL_DUPLEX:
                return SPI_DIRECTION_2LINES;
            case Direction::HALF_DUPLEX:
                return SPI_DIRECTION_1LINE;
            case Direction::SIMPLEX_TX_ONLY:
                return SPI_DIRECTION_2LINES_TXONLY;
            case Direction::SIMPLEX_RX_ONLY:
                return SPI_DIRECTION_2LINES_RXONLY;
        }
        return SPI_DIRECTION_2LINES;
    }

    static constexpr uint32_t translate_fifo_threshold(FIFOThreshold threshold) {
        // HAL uses (value-1) << 5 for FIFO threshold
        return (static_cast<uint32_t>(threshold) - 1) << 5;
    }

    static constexpr uint32_t translate_nss_polarity(NSSPolarity pol) {
        return (pol == NSSPolarity::ACTIVE_LOW) ? SPI_NSS_POLARITY_LOW : SPI_NSS_POLARITY_HIGH;
    }

    static constexpr uint32_t translate_nss_pulse(bool enable) {
        return enable ? SPI_NSS_PULSE_ENABLE : SPI_NSS_PULSE_DISABLE;
    }

    static constexpr uint32_t translate_ss_idleness(uint8_t cycles) {
        // Clamp to valid range 0-15
        if (cycles > 15) cycles = 15;
        return cycles;
    }

    static constexpr uint32_t translate_interdata_idleness(uint8_t cycles) {
        // Clamp to valid range 0-15, shift to proper position
        if (cycles > 15) cycles = 15;
        return (cycles << 4);  // These are in bits 7:4 of CFG2
    }

    static constexpr uint32_t translate_keep_io_state(bool enable) {
        return enable ? SPI_MASTER_KEEP_IO_STATE_ENABLE : SPI_MASTER_KEEP_IO_STATE_DISABLE;
    }

    static constexpr uint32_t translate_rx_autosusp(bool enable) {
        return enable ? SPI_MASTER_RX_AUTOSUSP_ENABLE : SPI_MASTER_RX_AUTOSUSP_DISABLE;
    }

    static constexpr uint32_t translate_io_swap(bool enable) {
        return enable ? SPI_IO_SWAP_ENABLE : SPI_IO_SWAP_DISABLE;
    }

    static constexpr uint32_t translate_ti_mode(bool enable) {
        return enable ? SPI_TIMODE_ENABLE : SPI_TIMODE_DISABLE;
    }

    static constexpr uint32_t translate_crc_calculation(bool enable) {
        return enable ? SPI_CRCCALCULATION_ENABLE : SPI_CRCCALCULATION_DISABLE;
    }

    static constexpr uint32_t translate_crc_length(CRCLength length) {
        if (length == CRCLength::DATASIZE) {
            return SPI_CRC_LENGTH_DATASIZE;
        }
        // HAL uses (value-1) << 16 for CRC length
        return (static_cast<uint32_t>(length) - 1) << 16;
    }
};

} // namespace ST_LIB

#endif // SPICONFIG_HPP
