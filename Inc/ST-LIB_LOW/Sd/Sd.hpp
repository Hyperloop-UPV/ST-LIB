/*
 * Sd.hpp
 *
 * Created on: 13 dec. 2025
 *         Author: Boris
 */

#ifndef SD_HPP
#define SD_HPP

#include "HALAL/HALAL.hpp"
#include "ST-LIB_LOW/DigitalInput2.hpp"
#include "stm32h7xx_hal.h"
#include "ErrorHandler/ErrorHandler.hpp"

using ST_LIB::DigitalInputDomain;
using ST_LIB::GPIODomain;

extern SD_HandleTypeDef* g_sdmmc1_handle;
extern SD_HandleTypeDef* g_sdmmc2_handle;
extern void* g_sdmmc1_instance_ptr;
extern void* g_sdmmc2_instance_ptr;

namespace ST_LIB {
struct SdDomain {

    enum class Peripheral : uint32_t {
        sdmmc1 = SDMMC1_BASE,
        sdmmc2 = SDMMC2_BASE,
    };


    struct Entry {
        Peripheral peripheral;
        std::size_t mpu_buffer0_idx;
        std::size_t mpu_buffer1_idx;
        std::optional<std::pair<size_t, GPIO_PinState>> cd_pin_idx; // Card Detect pin index in GPIO domain, if any
        std::optional<std::pair<size_t, GPIO_PinState>> wp_pin_idx; // Write Protect pin index in GPIO domain, if any
        std::size_t cmd_pin_idx;
        std::size_t ck_pin_idx;
        std::size_t d0_pin_idx; // Fixed for SDMMC2, configurable for SDMMC1
        std::size_t d1_pin_idx; // Fixed for SDMMC2, configurable for SDMMC1
        std::size_t d2_pin_idx;
        std::size_t d3_pin_idx;
    };


    template <std::size_t buffer_blocks>
    struct SdCard {
        using domain = SdDomain;
        Entry e;

        Peripheral peripheral;

        MPUDomain::Buffer<std::array<uint32_t, 512 * buffer_blocks / 4>> buffer0; // Alignment of 32-bit for SDMMC DMA
        MPUDomain::Buffer<std::array<uint32_t, 512 * buffer_blocks / 4>> buffer1; // Alignment of 32-bit for SDMMC DMA

        std::optional<std::pair<DigitalInputDomain::DigitalInput, GPIO_PinState>> cd; // Card Detect, if any, and its active state
        std::optional<std::pair<DigitalInputDomain::DigitalInput, GPIO_PinState>> wp; // Write Protect, if any, and its active state

        GPIODomain::GPIO cmd;
        GPIODomain::GPIO ck;
        GPIODomain::GPIO d0;
        GPIODomain::GPIO d1;
        GPIODomain::GPIO d2;
        GPIODomain::GPIO d3;

        /**
         * @brief Construct a new SdCard
         * @tparam buffer_blocks Number of 512-byte blocks for the MPU buffer
         * @param sdmmc_peripheral The SDMMC peripheral to use (Peripheral::sdmmc1 or Peripheral::sdmmc2)
         * @param card_detect_config Optional Card Detect pin (DigitalInputDomain::DigitalInput) and its active state, or nullopt for none
         * @param write_protect_config Optional Write Protect pin (DigitalInputDomain::DigitalInput) and its active state, or nullopt for none
         * @param d0_pin_for_sdmmc1 D0 pin to use if using SDMMC1 (default PC8, can also be PB13).
         * @note The other pins (CMD, CK, D1, D2, D3) are fixed for each peripheral.
         */
        consteval SdCard(Peripheral sdmmc_peripheral,
                    std::optional<std::pair<DigitalInputDomain::DigitalInput, GPIO_PinState>> card_detect_config, std::optional<std::pair<DigitalInputDomain::DigitalInput, GPIO_PinState>> write_protect_config,
                    GPIODomain::Pin d0_pin_for_sdmmc1 = ST_LIB::PC8) :
            e{.peripheral = sdmmc_peripheral},
            peripheral(sdmmc_peripheral),
            buffer0(MPUDomain::Buffer<std::array<uint32_t, 512 * buffer_blocks / 4>>(MPUDomain::MemoryType::NonCached, MPUDomain::MemoryDomain::D1)),
            buffer1(MPUDomain::Buffer<std::array<uint32_t, 512 * buffer_blocks / 4>>(MPUDomain::MemoryType::NonCached, MPUDomain::MemoryDomain::D1)),
            cd(card_detect_config),
            wp(write_protect_config),
            cmd((sdmmc_peripheral == Peripheral::sdmmc1) ? 
                GPIODomain::GPIO(ST_LIB::PD2, GPIODomain::OperationMode::ALT_PP, GPIODomain::Pull::None, GPIODomain::Speed::VeryHigh, GPIODomain::AlternateFunction::AF12) : 
                GPIODomain::GPIO(ST_LIB::PD7, GPIODomain::OperationMode::ALT_PP, GPIODomain::Pull::None, GPIODomain::Speed::VeryHigh, GPIODomain::AlternateFunction::AF11)),
            ck((sdmmc_peripheral == Peripheral::sdmmc1) ? 
                GPIODomain::GPIO(ST_LIB::PC12, GPIODomain::OperationMode::ALT_PP, GPIODomain::Pull::None, GPIODomain::Speed::VeryHigh, GPIODomain::AlternateFunction::AF12) : 
                GPIODomain::GPIO(ST_LIB::PD6, GPIODomain::OperationMode::ALT_PP, GPIODomain::Pull::None, GPIODomain::Speed::VeryHigh, GPIODomain::AlternateFunction::AF11)),
            d0((sdmmc_peripheral == Peripheral::sdmmc1) ? 
                GPIODomain::GPIO(d0_pin_for_sdmmc1, GPIODomain::OperationMode::ALT_PP, GPIODomain::Pull::None, GPIODomain::Speed::VeryHigh, GPIODomain::AlternateFunction::AF12) : 
                GPIODomain::GPIO(ST_LIB::PB14, GPIODomain::OperationMode::ALT_PP, GPIODomain::Pull::None, GPIODomain::Speed::VeryHigh, GPIODomain::AlternateFunction::AF9)),
            d1((sdmmc_peripheral == Peripheral::sdmmc1) ? 
                GPIODomain::GPIO(ST_LIB::PC9, GPIODomain::OperationMode::ALT_PP, GPIODomain::Pull::None, GPIODomain::Speed::VeryHigh, GPIODomain::AlternateFunction::AF12) : 
                GPIODomain::GPIO(ST_LIB::PB15, GPIODomain::OperationMode::ALT_PP, GPIODomain::Pull::None, GPIODomain::Speed::VeryHigh, GPIODomain::AlternateFunction::AF9)),
            d2((sdmmc_peripheral == Peripheral::sdmmc1) ? 
                GPIODomain::GPIO(ST_LIB::PC10, GPIODomain::OperationMode::ALT_PP, GPIODomain::Pull::None, GPIODomain::Speed::VeryHigh, GPIODomain::AlternateFunction::AF12) : 
                GPIODomain::GPIO(ST_LIB::PG11, GPIODomain::OperationMode::ALT_PP, GPIODomain::Pull::None, GPIODomain::Speed::VeryHigh, GPIODomain::AlternateFunction::AF10)),
            d3((sdmmc_peripheral == Peripheral::sdmmc1) ? 
                GPIODomain::GPIO(ST_LIB::PC11, GPIODomain::OperationMode::ALT_PP, GPIODomain::Pull::None, GPIODomain::Speed::VeryHigh, GPIODomain::AlternateFunction::AF12) : 
                GPIODomain::GPIO(ST_LIB::PG12, GPIODomain::OperationMode::ALT_PP, GPIODomain::Pull::None, GPIODomain::Speed::VeryHigh, GPIODomain::AlternateFunction::AF10))
        {
            if (sdmmc_peripheral != Peripheral::sdmmc1 && sdmmc_peripheral != Peripheral::sdmmc2) {
                ST_LIB::compile_error("Invalid SDMMC peripheral");
            }
            if ((d0_pin_for_sdmmc1.pin != ST_LIB::PC8.pin || d0_pin_for_sdmmc1.port != ST_LIB::PC8.port)
                 &&
                (d0_pin_for_sdmmc1.pin != ST_LIB::PB13.pin || d0_pin_for_sdmmc1.port != ST_LIB::PB13.port)) {
                ST_LIB::compile_error("D0 pin can only be PC8 or PB13 for SDMMC1");
            }
            if (buffer_blocks == 0) {
                ST_LIB::compile_error("Buffer blocks must be greater than 0");
            } else if (buffer_blocks > 15) {
                ST_LIB::compile_error("Buffer blocks must be less than or equal to 15");
            }
        }
    

        template <class Ctx>
        consteval void inscribe(Ctx &ctx) const {
            Entry local_e = e;

            local_e.mpu_buffer0_idx = ctx.template add<MPUDomain>(buffer0.e, this);
            local_e.mpu_buffer1_idx = ctx.template add<MPUDomain>(buffer1.e, this);

            if (cd.has_value()) {
                auto& di = cd.value().first;
                auto gpio_idx = ctx.template add<GPIODomain>(di.gpio.e);
                DigitalInputDomain::Entry di_entry{gpio_idx};
                local_e.cd_pin_idx = {ctx.template add<DigitalInputDomain>(di_entry), cd.value().second};
            }
            if (wp.has_value()) {
                auto& di = wp.value().first;
                auto gpio_idx = ctx.template add<GPIODomain>(di.gpio.e);
                DigitalInputDomain::Entry di_entry{gpio_idx};
                local_e.wp_pin_idx = {ctx.template add<DigitalInputDomain>(di_entry), wp.value().second};
            }

            local_e.cmd_pin_idx = ctx.template add<GPIODomain>(cmd.e, this);
            local_e.ck_pin_idx = ctx.template add<GPIODomain>(ck.e, this);
            local_e.d0_pin_idx = ctx.template add<GPIODomain>(d0.e, this);
            local_e.d1_pin_idx = ctx.template add<GPIODomain>(d1.e, this);
            local_e.d2_pin_idx = ctx.template add<GPIODomain>(d2.e, this);
            local_e.d3_pin_idx = ctx.template add<GPIODomain>(d3.e, this);

            ctx.template add<SdDomain>(local_e, this);
        }
    };


    static constexpr std::size_t max_instances = 2;


    struct Config {
        Peripheral peripheral;
        std::size_t mpu_buffer0_idx;
        std::size_t mpu_buffer1_idx;
        std::optional<std::pair<std::size_t, GPIO_PinState>> cd_pin_idx;
        std::optional<std::pair<std::size_t, GPIO_PinState>> wp_pin_idx;
        std::size_t cmd_pin_idx;
        std::size_t ck_pin_idx;
        std::size_t d0_pin_idx;
        std::size_t d1_pin_idx;
        std::size_t d2_pin_idx;
        std::size_t d3_pin_idx;
    };


    template <std::size_t N>
    static consteval std::array<Config, N> build(std::span<const Entry> entries) {
        std::array<Config, N> cfgs{};
        if (N == 0 ) {
            return cfgs;
        }
        bool peripheral_used[2] = {false, false}; // SDMMC1, SDMMC2

        for (std::size_t i = 0; i < N; i++) {
            const Entry &e = entries[i];

            // Verify uniqueness of peripheral usage
            std::size_t peripheral_index = (e.peripheral == Peripheral::sdmmc1) ? 0 : 1;
            if (peripheral_used[peripheral_index]) ST_LIB::compile_error("SDMMC peripheral used multiple times in SdDomain");
            peripheral_used[peripheral_index] = true;

            cfgs[i].peripheral = e.peripheral;
            cfgs[i].mpu_buffer0_idx = e.mpu_buffer0_idx;
            cfgs[i].mpu_buffer1_idx = e.mpu_buffer1_idx;
            cfgs[i].cd_pin_idx = e.cd_pin_idx;
            cfgs[i].wp_pin_idx = e.wp_pin_idx;
            cfgs[i].cmd_pin_idx = e.cmd_pin_idx;
            cfgs[i].ck_pin_idx = e.ck_pin_idx;
            cfgs[i].d0_pin_idx = e.d0_pin_idx;
            cfgs[i].d1_pin_idx = e.d1_pin_idx;
            cfgs[i].d2_pin_idx = e.d2_pin_idx;
            cfgs[i].d3_pin_idx = e.d3_pin_idx;
        }

        return cfgs;
    }

    enum class BufferSelect : bool {
        Buffer0 = false,
        Buffer1 = true
    };

    template <auto&> struct SdCardWrapper;
    template <std::size_t> struct Init;

    // State holder, logic is in SdCardWrapper
    struct Instance {
        template <auto &> friend struct SdCardWrapper;
        template <std::size_t N> friend struct Init;

        bool* operation_flag = nullptr; // External flag to indicate that an operation has finished. Only public so that it can be set in the public handlers below.

       // Public handlers called from C HAL callbacks. Don't use them, don't even think about using them.
       void on_dma_read_complete();
       void on_dma_write_complete();
       void on_abort();
       void on_error();

       private:
        SD_HandleTypeDef hsd;

        MPUDomain::Instance* mpu_buffer0_instance;
        MPUDomain::Instance* mpu_buffer1_instance;

        std::optional<std::pair<DigitalInputDomain::Instance*, GPIO_PinState>> cd_instance;
        std::optional<std::pair<DigitalInputDomain::Instance*, GPIO_PinState>> wp_instance;

        bool card_initialized;
        BufferSelect current_buffer; // The one that is currently available for CPU access and not being used by IDMA

        HAL_SD_CardInfoTypeDef card_info;

        // Functions
        bool is_card_present();
        bool is_write_protected();
        bool is_busy();
        bool initialize_card();
        bool deinitialize_card();
        void switch_buffer();
        bool configure_idma();
        // Variation of HAL_SDEx_ReadBlocksDMAMultiBuffer to fit our needs
        HAL_StatusTypeDef Not_HAL_SDEx_ReadBlocksDMAMultiBuffer(uint32_t BlockAdd, uint32_t NumberOfBlocks);
        HAL_StatusTypeDef Not_HAL_SDEx_WriteBlocksDMAMultiBuffer(uint32_t BlockAdd, uint32_t NumberOfBlocks);
    };

    template <auto &card_request>
    struct SdCardWrapper{
        static constexpr bool has_cd = card_request.cd.has_value();
        static constexpr bool has_wp = card_request.wp.has_value();
        
        SdCardWrapper(Instance& instance) : instance(instance) {
            check_cd_wp();
        };

        void init_card() {
            check_cd_wp();
            bool success = instance.initialize_card();
            if (!success) {
                ErrorHandler("SD Card initialization failed");
            }
        }

        void deinit_card() {
            check_cd_wp();
            bool success = instance.deinitialize_card();
            if (!success) {
                ErrorHandler("SD Card deinitialization failed");
            }
        }

        bool is_card_initialized() {
            return instance.card_initialized;
        }

        bool read_blocks(uint32_t start_block, uint32_t num_blocks, bool* operation_complete_flag) {
            check_cd_wp();
            if (!instance.card_initialized) {
                ErrorHandler("SD Card not initialized");
            }
            if (num_blocks > instance.mpu_buffer0_instance->size / 512) {
                ErrorHandler("Too many blocks requested to read from SD");
            }
            
            if (HAL_SD_GetCardState(&instance.hsd) != HAL_SD_CARD_TRANSFER) {
                return false; // Card not ready for data transfer
            }

            // Won't use HAL_SDEx_ReadBlocksDMAMultiBuffer because it doesn't support double buffering the way we want
            HAL_StatusTypeDef status = instance.Not_HAL_SDEx_ReadBlocksDMAMultiBuffer(start_block, num_blocks);

            if (status != HAL_OK) {
                ErrorHandler("SD Card read operation failed");
            }

            instance.operation_flag = operation_complete_flag;
            *operation_complete_flag = false;

            return true;
        }

        bool write_blocks(uint32_t start_block, uint32_t num_blocks, bool* operation_complete_flag) {
            check_cd_wp();
            if (!instance.card_initialized) {
                ErrorHandler("SD Card not initialized");
            }
            if (num_blocks > instance.mpu_buffer0_instance->size / 512) {
                ErrorHandler("Too many blocks requested to write in SD");
            }
            
            if (HAL_SD_GetCardState(&instance.hsd) != HAL_SD_CARD_TRANSFER) {
                return false; // Card not ready for data transfer
            }
            // Won't use HAL_SDEx_WriteBlocksDMAMultiBuffer because it doesn't support double buffering the way we want
            HAL_StatusTypeDef status = instance.Not_HAL_SDEx_WriteBlocksDMAMultiBuffer(start_block, num_blocks);

            if (status != HAL_OK) {
                ErrorHandler("SD Card write operation failed");
            }

            instance.operation_flag = operation_complete_flag;
            *operation_complete_flag = false;

            return true;
        }

        auto* get_current_buffer() {
            if (instance.current_buffer == BufferSelect::Buffer0) {
                return instance.mpu_buffer0_instance->template as<card_request.buffer0>();
            } else {
                return instance.mpu_buffer1_instance->template as<card_request.buffer1>();
            }
        }

        bool is_busy() {
            return instance.is_busy();
        }

       private:
        Instance& instance; // Actual State

        void check_cd_wp() {
            if constexpr (has_cd) {
                if (!instance.is_card_present()) { ErrorHandler("SD Card not present"); }
            }
            if constexpr (has_wp) {
                if (instance.is_write_protected()) { ErrorHandler("SD Card is write-protected"); }
            }
        }
    };


    template <std::size_t N>
    struct Init {
        static inline std::array<Instance, N> instances{};

        static void init(std::span<const Config, N> cfgs,
                         std::span<MPUDomain::Instance> mpu_buffer_instances,
                         std::span<DigitalInputDomain::Instance> digital_input_instances) {
            
            if (N == 0) {
                return;
            }

            for (std::size_t i = 0; i < N; i++) {
                const auto &cfg = cfgs[i];
                auto &inst = instances[i];

                inst.mpu_buffer0_instance = &mpu_buffer_instances[cfg.mpu_buffer0_idx];
                inst.mpu_buffer1_instance = &mpu_buffer_instances[cfg.mpu_buffer1_idx];

                if (cfg.cd_pin_idx.has_value()) {
                    inst.cd_instance = {&digital_input_instances[cfg.cd_pin_idx.value().first], cfg.cd_pin_idx.value().second};
                }
                if (cfg.wp_pin_idx.has_value()) {
                    inst.wp_instance = {&digital_input_instances[cfg.wp_pin_idx.value().first], cfg.wp_pin_idx.value().second};
                }

                if (cfg.peripheral == Peripheral::sdmmc1) {
                    inst.hsd.Instance = SDMMC1;
                } else if (cfg.peripheral == Peripheral::sdmmc2) {
                    inst.hsd.Instance = SDMMC2;
                }
                inst.hsd.Init.ClockEdge = SDMMC_CLOCK_EDGE_FALLING;
                inst.hsd.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
                inst.hsd.Init.BusWide = SDMMC_BUS_WIDE_4B;
                inst.hsd.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
                inst.hsd.Init.ClockDiv = 2;


                #ifdef SD_DEBUG_ENABLE
                // Doesn't really work in this moment, need to actually get the PLL1 frequency somehow
                inst.hsd.Init.BusWide = SDMMC_BUS_WIDE_1B;
                // Get a 400 kHz clock for debugging
                //uint32_t pll1_freq = HAL_RCCEx_GetPLL1ClockFreq(); // SDMMC clock source is PLL1
                uint32_t pll1_freq = 480000000; // Assume PLL1 is at 480 MHz
                uint32_t sdmmc_clk = pll1_freq / 2; // SDMMC clock before divider
                uint32_t target_div = sdmmc_clk / 400000; // Target divider
                if (target_div < 2) target_div = 2; // Minimum divider is 2
                if (target_div > 256) target_div = 256; // Maximum divider is 256
                inst.hsd.Init.ClockDiv = target_div - 2; // ClockDiv is (divider - 2)
                #endif // SD_DEBUG_ENABLE

                if (cfg.peripheral == Peripheral::sdmmc1) {
                    g_sdmmc1_handle = &inst.hsd;
                    g_sdmmc1_instance_ptr = &inst;
                } else if (cfg.peripheral == Peripheral::sdmmc2) {
                    g_sdmmc2_handle = &inst.hsd;
                    g_sdmmc2_instance_ptr = &inst;
                }

                inst.card_initialized = false;
                inst.current_buffer = BufferSelect::Buffer0;
            }

            // Initialize HAL SD
            RCC_PeriphCLKInitTypeDef RCC_PeriphCLKInitStruct;
            RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SDMMC;
            RCC_PeriphCLKInitStruct.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL;
            if (HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct) != HAL_OK) {
                ErrorHandler("SDMMC clock configuration failed, maybe try with a slower clock or higher divider?");
            }

            // Ensure PLL1Q output is enabled
            __HAL_RCC_PLLCLKOUT_ENABLE(RCC_PLL1_DIVQ);

            if (HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SDMMC) == 0) {
                ErrorHandler("SDMMC clock frequency is 0");
            }
        }
    };
};

static inline SdDomain::Instance* get_sd_instance(SD_HandleTypeDef* hsd) {
    if (hsd == g_sdmmc1_handle) return static_cast<SdDomain::Instance*>(g_sdmmc1_instance_ptr);
    if (hsd == g_sdmmc2_handle) return static_cast<SdDomain::Instance*>(g_sdmmc2_instance_ptr);
    return nullptr;
}

} // namespace ST_LIB

#endif // SD_HPP