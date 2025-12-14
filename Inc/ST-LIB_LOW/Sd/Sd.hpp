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

static void* g_sdmmc1_instance_ptr = nullptr;
static void* g_sdmmc2_instance_ptr = nullptr;

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
         * @param card_detect_config Optional Card Detect pin (DigitalInputDomain::DigitalInput) and its active state, or null for none
         * @param write_protect_config Optional Write Protect pin (DigitalInputDomain::DigitalInput) and its active state, or null for none
         * @param d0_pin_for_sdmmc1 D0 pin to use if using SDMMC1 (default PC8)
         * @param d1_pin_for_sdmmc1 D1 pin to use if using SDMMC1 (default PC9)
         * @note The other pins (CMD, CK, D2, D3) are fixed for each peripheral.
         */
        consteval SdCard(Peripheral sdmmc_peripheral,
                    std::optional<std::pair<DigitalInputDomain::DigitalInput, GPIO_PinState>> card_detect_config, std::optional<std::pair<DigitalInputDomain::DigitalInput, GPIO_PinState>> write_protect_config,
                    GPIODomain::Pin d0_pin_for_sdmmc1 = PC8, GPIODomain::Pin d1_pin_for_sdmmc1 = PC9) {

            e.peripheral = sdmmc_peripheral;

            buffer0 = MPUDomain::Buffer<std::array<uint32_t, 512 * buffer_blocks / 4>>(MPUDomain::MemoryType::NonCached, MPUDomain::MemoryDomain::D1);
            buffer1 = MPUDomain::Buffer<std::array<uint32_t, 512 * buffer_blocks / 4>>(MPUDomain::MemoryType::NonCached, MPUDomain::MemoryDomain::D1);

            cd = card_detect_config;
            wp = write_protect_config;

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
                e.cd_pin_idx = {ctx.template add<DigitalInputDomain>(cd.value().first), cd.value().second};
            }
            if (wp.has_value()) {
                e.wp_pin_idx = {ctx.template add<DigitalInputDomain>(wp.value().first), wp.value().second};
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

    enum class BufferSelect : bool {
        Buffer0 = false,
        Buffer1 = true
    };

    // State holder, logic is in SdCardWrapper
    struct Instance {
        friend struct SdCardWrapper;
        friend struct Init;

        bool* operation_flag = nullptr; // External flag to indicate that an operation has finished

       private:
        SD_HandleTypeDef hsd;

        MPUDomain::Instance* mpu_buffer0_instance;
        MPUDomain::Instance* mpu_buffer1_instance;

        std::optional<std::pair<DigitalInputDomain::Instance*, GPIO_PinState>> cd_instance;
        std::optional<std::pair<DigitalInputDomain::Instance*, GPIO_PinState>> wp_instance;

        bool card_initialized;
        BufferSelect current_buffer; // The one that is currently available for CPU access and not used by IDMA

        // Functions
        bool is_card_present() { return cd_instance->first->read() == cd_instance->second; }
        bool is_write_protected() { return wp_instance->first->read() == wp_instance->second; }

        bool is_busy() {
            return (hsd.State != HAL_SD_STATE_TRANSFER);
        }

        bool initialize_card() {
            if (card_initialized) { return true; } // Already initialized

            HAL_StatusTypeDef status = HAL_SD_Init(&hsd);

            if (status != HAL_OK) { return false; }

            if (HAL_SD_ConfigSpeedBusOperation(&hsd, SDMMC_SPEED_MODE_AUTO) != HAL_OK) {
                ErrorHandler("SD Card speed/bus configuration failed");
            }

            card_initialized = true;
            return true;
        }

        bool deinitialize_card() {
            if (!card_initialized) { return true; } // Already deinitialized

            HAL_StatusTypeDef status = HAL_SD_DeInit(&hsd);
            if (status != HAL_OK) { return false; }

            card_initialized = false;
            return true; // Placeholder
        }

        void switch_buffer() {
            current_buffer = (current_buffer == BufferSelect::Buffer0) ? BufferSelect::Buffer1 : BufferSelect::Buffer0;
        }

        bool configure_idma() {
            HAL_StatusTypeDef status = HAL_SDEx_ConfigDMAMultiBuffer(&hsd,
                reinterpret_cast<uint32_t*>(mpu_buffer0_instance->ptr),
                reinterpret_cast<uint32_t*>(mpu_buffer1_instance->ptr),
                mpu_buffer0_instance->size / 512); // Number of 512B-blocks

            if (status != HAL_OK) { return false; }
            return true;
        }
    };

    template <SdCard &card_request>
    struct SdCardWrapper{
        static constexpr bool has_cd = decltype(card_request)::cd.has_value();
        static constexpr bool has_wp = decltype(card_request)::wp.has_value();
        
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

        bool read_blocks(uint32_t start_block, uint32_t num_blocks, bool& operation_complete_flag) {
            check_cd_wp();
            if (!instance.card_initialized) {
                ErrorHandler("SD Card not initialized");
            }
            if (instance.is_busy()) {
                return false; // Busy
            }

            instance.operation_flag = &operation_complete_flag;
            operation_complete_flag = false;

            auto& buffer = get_current_buffer();

            // Won't use HAL_SDEx_ReadBlocksDMAMultiBuffer because it doesn't support double buffering the way we want
            HAL_StatusTypeDef status = Not_HAL_SDEx_ReadBlocksDMAMultiBuffer(start_block, num_blocks);

            if (status != HAL_OK) {
                ErrorHandler("SD Card read operation failed");
            }

            return true;
        }

        bool write_blocks(uint32_t start_block, uint32_t num_blocks, bool& operation_complete_flag) {
            check_cd_wp();
            if (!instance.card_initialized) {
                ErrorHandler("SD Card not initialized");
            }
            if (instance.is_busy()) {
                return false; // Busy
            }

            instance.operation_flag = &operation_complete_flag;
            operation_complete_flag = false;

            auto& buffer = get_current_buffer();

            // Won't use HAL_SDEx_WriteBlocksDMAMultiBuffer because it doesn't support double buffering the way we want
            HAL_StatusTypeDef status = Not_HAL_SDEx_WriteBlocksDMAMultiBuffer(start_block, num_blocks);

            if (status != HAL_OK) {
                ErrorHandler("SD Card write operation failed");
            }

            return true;
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

        auto& get_current_buffer() {
            if (instance.current_buffer == BufferSelect::Buffer0) {
                return instance.mpu_buffer0_instance->template as<card_request.buffer0>();
            } else {
                return instance.mpu_buffer1_instance->template as<card_request.buffer1>();
            }
        }

        // Variation of HAL_SDEx_ReadBlocksDMAMultiBuffer to fit our needs
        HAL_StatusTypeDef Not_HAL_SDEx_ReadBlocksDMAMultiBuffer(uint32_t BlockAdd, uint32_t NumberOfBlocks) {
            auto* hsd = instance.hsd;
            SDMMC_DataInitTypeDef config;
            uint32_t errorstate;
            uint32_t DmaBase0_reg;
            uint32_t DmaBase1_reg;
            uint32_t add = BlockAdd;

            if (hsd->State == HAL_SD_STATE_READY)
            {
                if ((add + NumberOfBlocks) > (hsd->SdCard.LogBlockNbr))
                {
                hsd->ErrorCode |= HAL_SD_ERROR_ADDR_OUT_OF_RANGE;
                return HAL_ERROR;
                }

                DmaBase0_reg = hsd->Instance->IDMABASE0;
                DmaBase1_reg = hsd->Instance->IDMABASE1;

                if ((hsd->Instance->IDMABSIZE == 0U) || (DmaBase0_reg == 0U) || (DmaBase1_reg == 0U))
                {
                hsd->ErrorCode = HAL_SD_ERROR_ADDR_OUT_OF_RANGE;
                return HAL_ERROR;
                }

                /* Initialize data control register */
                hsd->Instance->DCTRL = 0;
                /* Clear old Flags*/
                __HAL_SD_CLEAR_FLAG(hsd, SDMMC_STATIC_DATA_FLAGS);

                hsd->ErrorCode = HAL_SD_ERROR_NONE;
                hsd->State = HAL_SD_STATE_BUSY;

                if (hsd->SdCard.CardType != CARD_SDHC_SDXC)
                {
                add *= 512U;
                }

                /* Configure the SD DPSM (Data Path State Machine) */
                config.DataTimeOut   = SDMMC_DATATIMEOUT;
                config.DataLength    = BLOCKSIZE * NumberOfBlocks;
                config.DataBlockSize = SDMMC_DATABLOCK_SIZE_512B;
                config.TransferDir   = SDMMC_TRANSFER_DIR_TO_SDMMC;
                config.TransferMode  = SDMMC_TRANSFER_MODE_BLOCK;
                config.DPSM          = SDMMC_DPSM_DISABLE;
                (void)SDMMC_ConfigData(hsd->Instance, &config);

                hsd->Instance->DCTRL |= SDMMC_DCTRL_FIFORST;

                __SDMMC_CMDTRANS_ENABLE(hsd->Instance);

                if (instance.current_buffer == BufferSelect::Buffer1) {
                hsd->Instance->IDMACTRL = SDMMC_ENABLE_IDMA_DOUBLE_BUFF1;
                } else {
                hsd->Instance->IDMACTRL = SDMMC_ENABLE_IDMA_DOUBLE_BUFF0;
                }
                instance.switch_buffer();

                /* Read Blocks in DMA mode */
                hsd->Context = (SD_CONTEXT_READ_MULTIPLE_BLOCK | SD_CONTEXT_DMA);

                /* Read Multi Block command */
                errorstate = SDMMC_CmdReadMultiBlock(hsd->Instance, add);
                if (errorstate != HAL_SD_ERROR_NONE)
                {
                hsd->State = HAL_SD_STATE_READY;
                hsd->ErrorCode |= errorstate;
                return HAL_ERROR;
                }

                __HAL_SD_ENABLE_IT(hsd, (SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT | SDMMC_IT_RXOVERR | SDMMC_IT_DATAEND |
                                        SDMMC_IT_IDMABTC));

                return HAL_OK;
            }
            else
            {
                return HAL_BUSY;
            }

        }

        // Variation of HAL_SDEx_WriteBlocksDMAMultiBuffer to fit our needs
        HAL_StatusTypeDef Not_HAL_SDEx_WriteBlocksDMAMultiBuffer(uint32_t BlockAdd, uint32_t NumberOfBlocks) {
            auto* hsd = instance.hsd;
            SDMMC_DataInitTypeDef config;
            uint32_t errorstate;
            uint32_t DmaBase0_reg;
            uint32_t DmaBase1_reg;
            uint32_t add = BlockAdd;

            if (hsd->State == HAL_SD_STATE_READY)
            {
                if ((add + NumberOfBlocks) > (hsd->SdCard.LogBlockNbr))
                {
                hsd->ErrorCode |= HAL_SD_ERROR_ADDR_OUT_OF_RANGE;
                return HAL_ERROR;
                }

                DmaBase0_reg = hsd->Instance->IDMABASE0;
                DmaBase1_reg = hsd->Instance->IDMABASE1;
                if ((hsd->Instance->IDMABSIZE == 0U) || (DmaBase0_reg == 0U) || (DmaBase1_reg == 0U))
                {
                hsd->ErrorCode = HAL_SD_ERROR_ADDR_OUT_OF_RANGE;
                return HAL_ERROR;
                }

                /* Initialize data control register */
                hsd->Instance->DCTRL = 0;

                hsd->ErrorCode = HAL_SD_ERROR_NONE;

                hsd->State = HAL_SD_STATE_BUSY;

                if (hsd->SdCard.CardType != CARD_SDHC_SDXC)
                {
                add *= 512U;
                }

                /* Configure the SD DPSM (Data Path State Machine) */
                config.DataTimeOut   = SDMMC_DATATIMEOUT;
                config.DataLength    = BLOCKSIZE * NumberOfBlocks;
                config.DataBlockSize = SDMMC_DATABLOCK_SIZE_512B;
                config.TransferDir   = SDMMC_TRANSFER_DIR_TO_CARD;
                config.TransferMode  = SDMMC_TRANSFER_MODE_BLOCK;
                config.DPSM          = SDMMC_DPSM_DISABLE;
                (void)SDMMC_ConfigData(hsd->Instance, &config);

                //hsd->Instance->DCTRL |= SDMMC_DCTRL_FIFORST; // I am manually flushing the FIFO here, hal did not do it

                __SDMMC_CMDTRANS_ENABLE(hsd->Instance);

                //hsd->Instance->IDMACTRL = SDMMC_ENABLE_IDMA_DOUBLE_BUFF1;
                if (instance.current_buffer == BufferSelect::Buffer1) {
                hsd->Instance->IDMACTRL = SDMMC_ENABLE_IDMA_DOUBLE_BUFF1;
                } else {
                hsd->Instance->IDMACTRL = SDMMC_ENABLE_IDMA_DOUBLE_BUFF0;
                }
                instance.switch_buffer();

                /* Write Blocks in DMA mode */
                hsd->Context = (SD_CONTEXT_WRITE_MULTIPLE_BLOCK | SD_CONTEXT_DMA);

                /* Write Multi Block command */
                errorstate = SDMMC_CmdWriteMultiBlock(hsd->Instance, add);
                if (errorstate != HAL_SD_ERROR_NONE)
                {
                hsd->State = HAL_SD_STATE_READY;
                hsd->ErrorCode |= errorstate;
                return HAL_ERROR;
                }

                __HAL_SD_ENABLE_IT(hsd, (SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT | SDMMC_IT_TXUNDERR | SDMMC_IT_DATAEND |
                                        SDMMC_IT_IDMABTC));

                return HAL_OK;
            }
            else
            {
                return HAL_BUSY;
            }
            }
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
                inst.mpu_buffer0_instance = &mpu_buffer_instances[cfg.mpu_buffer0_idx];
                inst.mpu_buffer1_instance = &mpu_buffer_instances[cfg.mpu_buffer1_idx];
                if (!inst.configure_idma()) {
                    ErrorHandler("SD Card IDMA configuration failed");
                }

                if (cfg.cd_pin_idx.has_value()) {
                    inst.cd_instance = {&digital_input_instances[cfg.cd_pin_idx.value().first], cfg.cd_pin_idx.value().second};
                }
                if (cfg.wp_pin_idx.has_value()) {
                    inst.wp_instance = {&digital_input_instances[cfg.wp_pin_idx.value().first], cfg.wp_pin_idx.value().second};
                }

                inst.hsd.Instance = reinterpret_cast<SDMMC_TypeDef*>(static_cast<uintptr_t>(cfg.peripheral));
                inst.hsd.Init.ClockEdge = SDMMC_CLOCK_EDGE_FALLING;
                inst.hsd.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
                inst.hsd.Init.BusWide = SDMMC_BUS_WIDE_4B;
                inst.hsd.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
                inst.hsd.Init.ClockDiv = 0;


                #ifdef SD_DEBUG_ENABLE
                inst.hsd.Init.BusWide = SDMMC_BUS_WIDE_1B;
                // Get a 400 kHz clock for debugging
                uint32_t pll1_freq = HAL_RCCEx_GetPLL1ClockFreq(); // SDMMC clock source is PLL1
                uint32_t sdmmc_clk = pll1_freq / 2; // SDMMC clock before divider
                uint32_t target_div = sdmmc_clk / 400000; // Target divider
                if (target_div < 2) target_div = 2; // Minimum divider is 2
                if (target_div > 256) target_div = 256; // Maximum divider is 256
                inst.hsd.Init.ClockDiv = target_div - 2; // ClockDiv is (divider - 2)
                #endif // SD_DEBUG_ENABLE


                inst.card_initialized = false;
                inst.current_buffer = BufferSelect::Buffer0;

                // Initialize HAL SD
                RCC_PeriphCLKInitTypeDef RCC_PeriphCLKInitStruct;
                RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SDMMC;
                RCC_PeriphCLKInitStruct.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL;
                if (HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct) != HAL_OK) {
                    ErrorHandler("SDMMC1 clock configuration failed");
                }

                if (cfg.peripheral == Peripheral::sdmmc1) {
                    g_sdmmc1_handle = &inst.hsd;
                    g_sdmmc1_instance_ptr = &inst;
                    __HAL_RCC_SDMMC1_CLK_ENABLE();
                    HAL_NVIC_SetPriority(SDMMC1_IRQn, 5, 0);
                    HAL_NVIC_EnableIRQ(SDMMC1_IRQn);
                } else if (cfg.peripheral == Peripheral::sdmmc2) {
                    g_sdmmc2_handle = &inst.hsd;
                    g_sdmmc2_instance_ptr = &inst;
                    __HAL_RCC_SDMMC2_CLK_ENABLE();
                    HAL_NVIC_SetPriority(SDMMC2_IRQn, 5, 0);
                    HAL_NVIC_EnableIRQ(SDMMC2_IRQn);
                }
            }
        }
    };
};

static inline SdDomain::Instance* get_sd_instance(SD_HandleTypeDef* hsd) {
    if (hsd == g_sdmmc1_handle) return static_cast<SdDomain::Instance*>(g_sdmmc1_instance_ptr);
    if (hsd == g_sdmmc2_handle) return static_cast<SdDomain::Instance*>(g_sdmmc2_instance_ptr);
    return nullptr;
}

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

extern "C" {

void HAL_SDEx_Read_DMADoubleBuf0CpltCallback(SD_HandleTypeDef* hsd) {
    auto sd_instance = get_sd_instance(hsd);
    if (sd_instance && sd_instance->operation_flag) {
        *sd_instance->operation_flag = true;
        sd_instance->operation_flag = nullptr;
    }
    SDMMC_CmdStopTransfer(hsd->Instance);
}
void HAL_SDEx_Read_DMADoubleBuf1CpltCallback(SD_HandleTypeDef* hsd) {
    auto sd_instance = get_sd_instance(hsd);
    if (sd_instance && sd_instance->operation_flag) {
        *sd_instance->operation_flag = true;
        sd_instance->operation_flag = nullptr;
    }
    SDMMC_CmdStopTransfer(hsd->Instance);
}

void HAL_SDEx_Write_DMADoubleBuf0CpltCallback(SD_HandleTypeDef* hsd) {
    auto sd_instance = get_sd_instance(hsd);
    if (sd_instance && sd_instance->operation_flag) {
        *sd_instance->operation_flag = true;
        sd_instance->operation_flag = nullptr;
    }
    SDMMC_CmdStopTransfer(hsd->Instance);
}
void HAL_SDEx_Write_DMADoubleBuf1CpltCallback(SD_HandleTypeDef* hsd) {
    auto sd_instance = get_sd_instance(hsd);
    if (sd_instance && sd_instance->operation_flag) {
        *sd_instance->operation_flag = true;
        sd_instance->operation_flag = nullptr;
    }
    SDMMC_CmdStopTransfer(hsd->Instance);
}

void HAL_SD_AbortCallback(SD_HandleTypeDef* hsd) {
    // auto sd_instance = get_sd_instance(hsd);
    ErrorHandler("SD Card operation aborted");
}

void HAL_SD_ErrorCallback(SD_HandleTypeDef* hsd) {
    //auto sd_instance = get_sd_instance(hsd);
    ErrorHandler("SD Card error occurred");
}

} // extern "C"

#endif // SD_HPP