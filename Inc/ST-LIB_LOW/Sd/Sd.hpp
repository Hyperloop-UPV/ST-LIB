/*
 * Sd.hpp
 *
 * Created on: 13 dec. 2025
 *         Author: Boris
 */

#ifndef SD_HPP
#define SD_HPP

#include "HALAL/HALAL.hpp"
#include "ST-LIB_LOW/ST-LIB_LOW.hpp"
#include "stm32h7xx_hal.h"
#include "ErrorHandler/ErrorHandler.hpp"

using ST_LIB::DigitalInputDomain;
using ST_LIB::GPIODomain;

static SD_HandleTypeDef* g_sdmmc1_handle = nullptr;
static SD_HandleTypeDef* g_sdmmc2_handle = nullptr;

struct SdDomain {

    enum class Peripheral : uint32_t {
        sdmmc1 = SDMMC1_BASE,
        sdmmc2 = SDMMC2_BASE,
    };


    struct Entry {
        Peripheral peripheral;
        std::size_t mpu_buffer0_idx;
        std::size_t mpu_buffer1_idx;
        std::optional<size_t> cd_pin_idx; // Card Detect pin index in GPIO domain, if any
        std::optional<size_t> wp_pin_idx; // Write Protect pin index in GPIO domain, if any
        std::size_t cmd_pin_idx;
        std::size_t ck_pin_idx;
        std::size_t d0_pin_idx; // Hardcoded unless SDMMC1
        std::size_t d1_pin_idx; // Hardcoded unless SDMMC1
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

        std::optional<DigitalInputDomain::DigitalInput> cd; // Card Detect, if any
        std::optional<DigitalInputDomain::DigitalInput> wp; // Write Protect, if any

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
         * @param card_detect Optional Card Detect pin (DigitalInputDomain::DigitalInput), or null for none
         * @param write_protect Optional Write Protect pin (DigitalInputDomain::DigitalInput), or null for none
         * @param d0_pin_for_sdmmc1 D0 pin to use if using SDMMC1 (default PC8)
         * @param d1_pin_for_sdmmc1 D1 pin to use if using SDMMC1 (default PC9)
         * @note The other pins (CMD, CK, D2, D3) are fixed for each peripheral.
         */
        consteval SdCard(Peripheral sdmmc_peripheral,
                    std::optional<DigitalInputDomain::DigitalInput> card_detect, std::optional<DigitalInputDomain::DigitalInput> write_protect,
                    GPIODomain::Pin d0_pin_for_sdmmc1 = PC8, GPIODomain::Pin d1_pin_for_sdmmc1 = PC9) {

            e.peripheral = sdmmc_peripheral;

            buffer0 = MPUDomain::Buffer<std::array<uint32_t, 512 * buffer_blocks / 4>>();
            buffer1 = MPUDomain::Buffer<std::array<uint32_t, 512 * buffer_blocks / 4>>();

            cd = card_detect;
            wp = write_protect;

            if (sdmmc_peripheral == Peripheral::sdmmc1) {
                cmd = GPIODomain::GPIO(PC6, GPIODomain::OperationMode::ALT_PP, GPIODomain::Pull::None, GPIODomain::Speed::VeryHigh, GPIODomain::AlternateFunction::AF12);
                ck = GPIODomain::GPIO(PC12, GPIODomain::OperationMode::ALT_PP, GPIODomain::Pull::None, GPIODomain::Speed::VeryHigh, GPIODomain::AlternateFunction::AF12);
                d0 = GPIODomain::GPIO(d0_pin_for_sdmmc1, GPIODomain::OperationMode::ALT_PP, GPIODomain::Pull::None, GPIODomain::Speed::VeryHigh, GPIODomain::AlternateFunction::AF12);
                d1 = GPIODomain::GPIO(d1_pin_for_sdmmc1, GPIODomain::OperationMode::ALT_PP, GPIODomain::Pull::None, GPIODomain::Speed::VeryHigh, GPIODomain::AlternateFunction::AF12);
                d2 = GPIODomain::GPIO(PC10, GPIODomain::OperationMode::ALT_PP, GPIODomain::Pull::None, GPIODomain::Speed::VeryHigh, GPIODomain::AlternateFunction::AF12);
                d3 = GPIODomain::GPIO(PC11, GPIODomain::OperationMode::ALT_PP, GPIODomain::Pull::None, GPIODomain::Speed::VeryHigh, GPIODomain::AlternateFunction::AF12);
            } else if (sdmmc_peripheral == Peripheral::sdmmc2) {
                cmd = GPIODomain::GPIO(PD7, GPIODomain::OperationMode::ALT_PP, GPIODomain::Pull::None, GPIODomain::Speed::VeryHigh, GPIODomain::AlternateFunction::AF12);
                ck = GPIODomain::GPIO(PD6, GPIODomain::OperationMode::ALT_PP, GPIODomain::Pull::None, GPIODomain::Speed::VeryHigh, GPIODomain::AlternateFunction::AF12);
                d0 = GPIODomain::GPIO(PB14, GPIODomain::OperationMode::ALT_PP, GPIODomain::Pull::None, GPIODomain::Speed::VeryHigh, GPIODomain::AlternateFunction::AF12);
                d1 = GPIODomain::GPIO(PB15, GPIODomain::OperationMode::ALT_PP, GPIODomain::Pull::None, GPIODomain::Speed::VeryHigh, GPIODomain::AlternateFunction::AF12);
                d2 = GPIODomain::GPIO(PG11, GPIODomain::OperationMode::ALT_PP, GPIODomain::Pull::None, GPIODomain::Speed::VeryHigh, GPIODomain::AlternateFunction::AF12);
                d3 = GPIODomain::GPIO(PG12, GPIODomain::OperationMode::ALT_PP, GPIODomain::Pull::None, GPIODomain::Speed::VeryHigh, GPIODomain::AlternateFunction::AF12);
            } else {
                throw "Invalid SDMMC peripheral";
            }
        }
    

        template <class Ctx>
        consteval void inscribe(Ctx &ctx) const {
            e.mpu_buffer0_idx = ctx.template add<MPUDomain>(buffer0);
            e.mpu_buffer1_idx = ctx.template add<MPUDomain>(buffer1);

            if (cd.has_value()) {
                e.cd_pin_idx = ctx.template add<DigitalInputDomain>(cd.value());
            }
            if (wp.has_value()) {
                e.wp_pin_idx = ctx.template add<DigitalInputDomain>(wp.value());
            }

            e.cmd_pin_idx = ctx.template add<GPIODomain>(cmd);
            e.ck_pin_idx = ctx.template add<GPIODomain>(ck);
            e.d0_pin_idx = ctx.template add<GPIODomain>(d0);
            e.d1_pin_idx = ctx.template add<GPIODomain>(d1);
            e.d2_pin_idx = ctx.template add<GPIODomain>(d2);
            e.d3_pin_idx = ctx.template add<GPIODomain>(d3);

            ctx.template add<SdDomain>(e);
        }
    };


    static constexpr std::size_t max_instances = 2;


    struct Config {
        Peripheral peripheral;
        std::size_t mpu_buffer0_idx;
        std::size_t mpu_buffer1_idx;
        std::optional<size_t> cd_pin_idx;
        std::optional<size_t> wp_pin_idx;
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
        bool peripheral_used[2] = {false, false}; // SDMMC1, SDMMC2

        for (std::size_t i = 0; i < N; i++) {
            const Entry &e = entries[i];

            // Verify uniqueness of peripheral usage
            std::size_t peripheral_index = (e.peripheral == Peripheral::sdmmc1) ? 0 : 1;
            if (peripheral_used[peripheral_index]) throw "SDMMC peripheral already used";
            peripheral_used[peripheral_index] = true;

            // Fill configuration
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


    // State holder, logic is in SdCardWrapper
    struct Instance {
        friend class SdCardWrapper;
        friend struct Init;

       private:
        SD_HandleTypeDef hsd;

        MPUDomain::Instance* mpu_buffer0_instance;
        MPUDomain::Instance* mpu_buffer1_instance;

        std::optional<DigitalInputDomain::Instance*> cd_instance;
        std::optional<DigitalInputDomain::Instance*> wp_instance;

        bool card_initialized;
    };

    struct SdCardWrapperI {

    };

    template <SdCard &card_request>
    struct SdCardWrapper : public SdCardWrapperI {
        using peripheral = decltype(card_request)::peripheral;
        static constexpr bool has_cd = decltype(card_request)::cd.has_value();
        static constexpr bool has_wp = decltype(card_request)::wp.has_value();
        
        SdCardWrapper(Instance& instance) : instance(instance) {};

        // Methods to operate the SD card

       private:
        Instance& instance; // Actual State
    };


    template <std::size_t N>
    struct Init {
        static inline std::array<Instance, N> instances{};

        static void init(std::span<const Config, N> cfgs,
                         std::span<MPUDomain::Instance> mpu_buffer_instances,
                         std::span<DigitalInputDomain::Instance> digital_input_instances) {

            for (std::size_t i = 0; i < N; i++) {
                const auto &cfg = cfgs[i];
                auto &inst = instances[i];
                inst.mpu_buffer0_instance = mpu_buffer_instances[cfg.mpu_buffer0_idx];
                inst.mpu_buffer1_instance = mpu_buffer_instances[cfg.mpu_buffer1_idx];
                if (cfg.cd_pin_idx.has_value()) {
                    inst.cd_instance = &digital_input_instances[cfg.cd_pin_idx.value()];
                }
                if (cfg.wp_pin_idx.has_value()) {
                    inst.wp_instance = &digital_input_instances[cfg.wp_pin_idx.value()];
                }

                inst.hsd.Instance = reinterpret_cast<SDMMC_TypeDef*>(static_cast<uintptr_t>(cfg.peripheral));
                inst.card_initialized = false;

                // Initialize HAL SD
                RCC_PeriphCLKInitTypeDef RCC_PeriphCLKInitStruct;
                RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SDMMC;
                RCC_PeriphCLKInitStruct.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL;
                if (HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct) != HAL_OK) {
                    ErrorHandler("SDMMC1 clock configuration failed");
                }

                if (cfg.peripheral == Peripheral::sdmmc1) {
                    g_sdmmc1_handle = &inst.hsd;
                    __HAL_RCC_SDMMC1_CLK_ENABLE();
                    HAL_NVIC_SetPriority(SDMMC1_IRQn, 5, 0);
                    HAL_NVIC_EnableIRQ(SDMMC1_IRQn);
                } else if (cfg.peripheral == Peripheral::sdmmc2) {
                    g_sdmmc2_handle = &inst.hsd;
                    __HAL_RCC_SDMMC2_CLK_ENABLE();
                    HAL_NVIC_SetPriority(SDMMC2_IRQn, 5, 0);
                    HAL_NVIC_EnableIRQ(SDMMC2_IRQn);
                }
            }
        }
    };
};


extern "C" void SDMMC1_IRQHandler(void) {
    if (g_sdmmc1_handle != nullptr) {
        HAL_SD_IRQHandler(g_sdmmc1_handle);
    }
}

extern "C" void SDMMC2_IRQHandler(void) {
    if (g_sdmmc2_handle != nullptr) {
        HAL_SD_IRQHandler(g_sdmmc2_handle);
    }
}

void HAL_SDEx_Read_DMADoubleBuf0CpltCallback(SD_HandleTypeDef* hsd) {
    //auto sd_instance = reinterpret_cast<SdDomain::SdCardWrapperI*>(hsd->Context);
    //SDMMC_CmdStopTransfer(hsd->Instance);
}
void HAL_SDEx_Read_DMADoubleBuf1CpltCallback(SD_HandleTypeDef* hsd) {
    //auto sd_instance = reinterpret_cast<SdDomain::SdCardWrapperI*>(hsd->Context);
    //SDMMC_CmdStopTransfer(hsd->Instance);
}

void HAL_SDEx_Write_DMADoubleBuf0CpltCallback(SD_HandleTypeDef* hsd) {
    //auto sd_instance = reinterpret_cast<SdDomain::SdCardWrapperI*>(hsd->Context);
    //SDMMC_CmdStopTransfer(hsd->Instance);
}
void HAL_SDEx_Write_DMADoubleBuf1CpltCallback(SD_HandleTypeDef* hsd) {
    //auto sd_instance = reinterpret_cast<SdDomain::SdCardWrapperI*>(hsd->Context);
    //SDMMC_CmdStopTransfer(hsd->Instance);
}

void HAL_SD_AbortCallback(SD_HandleTypeDef* hsd) {
    //auto sd_instance = reinterpret_cast<SdDomain::SdCardWrapperI*>(hsd->Context);
}

void HAL_SD_ErrorCallback(SD_HandleTypeDef* hsd) {
    //auto sd_instance = reinterpret_cast<SdDomain::SdCardWrapperI*>(hsd->Context);
    ErrorHandler("SD Card error occurred");
}

#endif // SD_HPP