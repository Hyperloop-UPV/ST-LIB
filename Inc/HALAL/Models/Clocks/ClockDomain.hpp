#pragma once

#include <cstdint>
#include <type_traits>
#include <array>
#include <span>
#include "ST-LIB_LOW/ErrorHandler/ErrorHandler.hpp"

namespace ST_LIB {
extern void compile_error(const char* msg);

struct ClockDomain {

    /**
     * =========================================
     *              Inner Workings
     * =========================================
     */

    enum class ClockGroup : uint8_t {
        SPI123_G,
        SPI45_G,
        SPI6_G,
        SDMMC_G,
        ADC_G,
        FDCAN_G,
        USART16_G,
        USART234578_G,
        I2C1235_G,
        I2C4_G,
        LPUART1_G,
        LPTIM_G,
        RNG_G,
        USB_G,
        DFSDM1_G,
        SAI1_G,
        SAI4_G,
        ETH_MAC_G,
        RTC_G,
        CEC_G,
        APB1_TIM_G,
        APB2_TIM_G,
        D1_BUS_G,
        D2_BUS_G,
        D3_BUS_G,
    };

    static constexpr uint32_t PLLM_MIN = 1;
    static constexpr uint32_t PLLM_MAX = 63;
    static constexpr uint32_t PLLN_MIN = 4;
    static constexpr uint32_t PLLN_MAX = 512;
    static constexpr uint32_t PLLP_MIN = 1;
    static constexpr uint32_t PLLP_MAX = 128;
    static constexpr uint32_t PLLQ_MIN = 1;
    static constexpr uint32_t PLLQ_MAX = 128;
    static constexpr uint32_t PLLR_MIN = 1;
    static constexpr uint32_t PLLR_MAX = 128;

    static consteval bool is_valid_pll1p(uint32_t p) {
        return p == PLLP_MIN ||
               (p <= PLLP_MAX && p % 2 == 0); // 1 is bypass, then even numbers up to 128
    }

    static constexpr uint32_t PLL_IN_MIN = 1'000'000;
    static constexpr uint32_t PLL_IN_MAX = 16'000'000;

    static constexpr uint32_t VCOH_MIN = 192'000'000;
    static constexpr uint32_t VCOH_MAX = 836'000'000;
    static constexpr uint32_t VCOL_MIN = 150'000'000;
    static constexpr uint32_t VCOL_MAX = 420'000'000;

    static constexpr uint32_t SYSCLK_MAX = 550'000'000;
    static constexpr uint32_t HCLK_MAX = 275'000'000;

    static constexpr uint32_t d1cpre_values[] = {1, 2, 4, 8, 16, 64, 128, 256, 512};

    struct ClockTree {
        uint32_t hse_frequency = 0;
        bool hse_bypass = false;

        uint32_t pll1_m = 0;
        uint32_t pll1_n = 0;
        uint32_t pll1_p = 0;
        uint32_t pll1_q = 0;
        uint32_t pll1_r = 0;
        uint32_t pll1_fracn = 0;

        uint32_t pll2_m = 0;
        uint32_t pll2_n = 0;
        uint32_t pll2_p = 0;
        uint32_t pll2_q = 0;
        uint32_t pll2_r = 0;
        uint32_t pll2_fracn = 0;

        uint32_t pll3_m = 0;
        uint32_t pll3_n = 0;
        uint32_t pll3_p = 0;
        uint32_t pll3_q = 0;
        uint32_t pll3_r = 0;
        uint32_t pll3_fracn = 0;

        uint32_t d1cpre = 1;
        uint32_t d2ppre1 = 2;
        uint32_t d2ppre2 = 2;

        enum class Source : uint8_t {
            None,
            HSI,
            HSE,
            CSI,
            PCLK1,
            PCLK2,
            PLL1Q,
            PLL1R,
            PLL2P,
            PLL2Q,
            PLL2R,
            PLL3P,
            PLL3Q,
            PLL3R,
        };
        Source spi123_src = Source::None;
        Source spi45_src = Source::None;
        Source spi6_src = Source::None;
        Source adc_src = Source::None;
        Source fdcan_src = Source::None;
        Source sdmmc_src = Source::None;
    };

    static constexpr uint32_t find_rge(uint32_t pll_input) {
        if (pll_input >= 1'000'000 && pll_input <= 2'000'000)
            return 0;
        if (pll_input > 2'000'000 && pll_input <= 4'000'000)
            return 1;
        if (pll_input > 4'000'000 && pll_input <= 8'000'000)
            return 2;
        if (pll_input > 8'000'000 && pll_input <= 16'000'000)
            return 3;
        return 0;
    }

    static constexpr uint32_t flash_ws(uint32_t hclk) {
        if (hclk <= 70'000'000)
            return 1;
        if (hclk <= 140'000'000)
            return 2;
        if (hclk <= 210'000'000)
            return 3;
        return 4;
    }

    static constexpr uint32_t pll1_input(const ClockTree& t) { return t.hse_frequency / t.pll1_m; }
    static constexpr uint32_t pll2_input(const ClockTree& t) { return t.hse_frequency / t.pll2_m; }
    static constexpr uint32_t pll3_input(const ClockTree& t) { return t.hse_frequency / t.pll3_m; }

    static constexpr uint32_t pll1_vco(const ClockTree& t) {
        uint32_t in = pll1_input(t);
        return in * t.pll1_n + in * t.pll1_fracn / 8192;
    }
    static constexpr uint32_t pll2_vco(const ClockTree& t) {
        uint32_t in = pll2_input(t);
        return in * t.pll2_n + in * t.pll2_fracn / 8192;
    }
    static constexpr uint32_t pll3_vco(const ClockTree& t) {
        uint32_t in = pll3_input(t);
        return in * t.pll3_n + in * t.pll3_fracn / 8192;
    }

    static constexpr bool pll1_vco_wide(const ClockTree& t) { return pll1_input(t) >= 2'000'000; }
    static constexpr uint32_t pll1_rge(const ClockTree& t) { return find_rge(pll1_input(t)); }
    static constexpr uint32_t pll2_rge(const ClockTree& t) { return find_rge(pll2_input(t)); }
    static constexpr uint32_t pll3_rge(const ClockTree& t) { return find_rge(pll3_input(t)); }

    static constexpr uint32_t sysclk(const ClockTree& t) { return pll1_vco(t) / t.pll1_p; }
    static constexpr uint32_t hclk(const ClockTree& t) { return sysclk(t) / t.d1cpre; }
    static constexpr uint32_t pclk1(const ClockTree& t) { return hclk(t) / t.d2ppre1; }
    static constexpr uint32_t pclk2(const ClockTree& t) { return hclk(t) / t.d2ppre2; }
    static constexpr uint32_t timer_apb1(const ClockTree& t) {
        return pclk1(t) * (t.d2ppre1 == 1 ? 1 : 2);
    }
    static constexpr uint32_t timer_apb2(const ClockTree& t) {
        return pclk2(t) * (t.d2ppre2 == 1 ? 1 : 2);
    }
    static constexpr uint32_t flash_latency(const ClockTree& t) { return flash_ws(hclk(t)); }

    static constexpr uint32_t source_frequency(const ClockTree& t, ClockTree::Source src) {
        switch (src) {
        case ClockTree::Source::None:
            return 0;
        case ClockTree::Source::HSI:
            return 64'000'000;
        case ClockTree::Source::HSE:
            return t.hse_frequency;
        case ClockTree::Source::CSI:
            return 4'000'000;
        case ClockTree::Source::PCLK1:
            return pclk1(t);
        case ClockTree::Source::PCLK2:
            return pclk2(t);
        case ClockTree::Source::PLL1Q:
            return pll1_vco(t) / t.pll1_q;
        case ClockTree::Source::PLL1R:
            return pll1_vco(t) / t.pll1_r;
        case ClockTree::Source::PLL2P:
            return t.pll2_p != 0 ? pll2_vco(t) / t.pll2_p : 0;
        case ClockTree::Source::PLL2Q:
            return t.pll2_q != 0 ? pll2_vco(t) / t.pll2_q : 0;
        case ClockTree::Source::PLL2R:
            return t.pll2_r != 0 ? pll2_vco(t) / t.pll2_r : 0;
        case ClockTree::Source::PLL3P:
            return t.pll3_p != 0 ? pll3_vco(t) / t.pll3_p : 0;
        case ClockTree::Source::PLL3Q:
            return t.pll3_q != 0 ? pll3_vco(t) / t.pll3_q : 0;
        case ClockTree::Source::PLL3R:
            return t.pll3_r != 0 ? pll3_vco(t) / t.pll3_r : 0;
        default:
            break;
        }
        if consteval {
            compile_error("Unknown clock source");
        } else {
            PANIC("Unknown clock source");
        }
        return 0;
    }

    static constexpr uint32_t get_kernel_clock(const ClockTree& t, ClockGroup group) {
        switch (group) {
        case ClockGroup::SPI123_G:
            return source_frequency(t, t.spi123_src);
        case ClockGroup::SPI45_G:
            return source_frequency(t, t.spi45_src);
        case ClockGroup::SPI6_G:
            return source_frequency(t, t.spi6_src);
        case ClockGroup::ADC_G:
            return source_frequency(t, t.adc_src);
        case ClockGroup::FDCAN_G:
            return source_frequency(t, t.fdcan_src);
        case ClockGroup::SDMMC_G:
            return source_frequency(t, t.sdmmc_src);
        case ClockGroup::APB1_TIM_G:
            return timer_apb1(t);
        case ClockGroup::APB2_TIM_G:
            return timer_apb2(t);
        case ClockGroup::D1_BUS_G:
            return sysclk(t);
        case ClockGroup::D2_BUS_G:
            return pclk1(t);
        case ClockGroup::D3_BUS_G:
            return pclk2(t);
        default:
            break;
        }
        if consteval {
            compile_error("Unknown clock group");
        } else {
            PANIC("Unknown clock group");
        }
        return 0;
    }

    /**
     * =========================================
     *              Domain Contract
     * =========================================
     */

    static inline const ClockTree* s_tree = nullptr;

    static uint32_t pll1_input() { return pll1_input(*s_tree); }
    static uint32_t pll1_vco() { return pll1_vco(*s_tree); }
    static uint32_t sysclk() { return sysclk(*s_tree); }
    static uint32_t hclk() { return hclk(*s_tree); }
    static uint32_t pclk1() { return pclk1(*s_tree); }
    static uint32_t pclk2() { return pclk2(*s_tree); }
    static uint32_t timer_apb1() { return timer_apb1(*s_tree); }
    static uint32_t timer_apb2() { return timer_apb2(*s_tree); }
    static uint32_t source_frequency(ClockTree::Source src) {
        return source_frequency(*s_tree, src);
    }
    static uint32_t get_kernel_clock(ClockGroup group) { return get_kernel_clock(*s_tree, group); }

    static constexpr std::size_t max_instances = 32;

    struct Entry {
        ClockGroup group;
        using TrySolveFn = bool (*)(uint32_t kernel_clk);
        TrySolveFn try_solve;
    };

    struct Device {
        ClockGroup group;
        Entry::TrySolveFn try_solve;

        template <typename Ctx> constexpr std::size_t inscribe(Ctx& ctx) const {
            if (try_solve == nullptr) {
                compile_error("ClockDomain::Device: try_solve function pointer is null");
            }
            return ctx.template add<ClockDomain>(
                Entry{
                    .group = group,
                    .try_solve = try_solve,
                },
                this
            );
        }
    };

    static consteval void validate(const ClockTree& t, std::span<const Entry> entries) {
        if (t.hse_frequency == 0)
            compile_error("HSE frequency is zero");
        if (t.pll1_m == 0)
            compile_error("PLL1M is zero");

        uint32_t pll_in = pll1_input(t);
        uint32_t vco = pll1_vco(t);
        uint32_t sclk = sysclk(t);
        uint32_t h_clk = hclk(t);

        if (sclk > SYSCLK_MAX)
            compile_error("SYSCLK exceeds maximum (550 MHz)");
        if (h_clk > HCLK_MAX)
            compile_error("HCLK exceeds maximum (275 MHz)");
        if (pll_in < PLL_IN_MIN || pll_in > PLL_IN_MAX)
            compile_error("PLL1 input outside valid range (1-16 MHz)");
        if (t.pll1_m < PLLM_MIN || t.pll1_m > PLLM_MAX)
            compile_error("PLL1M out of range");
        if (t.pll1_n < PLLN_MIN || t.pll1_n > PLLN_MAX)
            compile_error("PLL1N out of range");
        if (!is_valid_pll1p(t.pll1_p))
            compile_error("Invalid PLL1P divider");

        if (pll1_vco_wide(t)) {
            if (vco < VCOH_MIN || vco > VCOH_MAX)
                compile_error("PLL1 VCO outside wide range (192-836 MHz)");
        } else {
            if (vco < VCOL_MIN || vco > VCOL_MAX)
                compile_error("PLL1 VCO outside medium range (150-420 MHz)");
        }

        bool d1cpre_valid = false;
        for (auto d : d1cpre_values) {
            if (t.d1cpre == d) {
                d1cpre_valid = true;
                break;
            }
        }
        if (!d1cpre_valid)
            compile_error("D1CPRE is not a valid divider");

        // Peripheral requirements
        for (size_t i = 0; i < entries.size(); i++) {
            const auto& ent = entries[i];
            uint32_t ker = get_kernel_clock(t, ent.group);
            if (ker == 0)
                compile_error("No kernel clock assigned for peripheral group");
            if (!ent.try_solve(ker))
                compile_error("Kernel clock does not satisfy peripheral requirements");
        }

        // PLL2/3 source consistency
        if (t.spi123_src != ClockTree::Source::None) {
            if (t.spi123_src == ClockTree::Source::PLL2P && (t.pll2_m == 0 || t.pll2_p == 0))
                compile_error("SPI123 uses PLL2P but PLL2 not configured");
            if (t.spi123_src == ClockTree::Source::PLL3P && (t.pll3_m == 0 || t.pll3_p == 0))
                compile_error("SPI123 uses PLL3P but PLL3 not configured");
        }
        if (t.spi45_src != ClockTree::Source::None) {
            if (t.spi45_src == ClockTree::Source::PLL2Q && (t.pll2_m == 0 || t.pll2_q == 0))
                compile_error("SPI45 uses PLL2Q but PLL2 not configured");
            if (t.spi45_src == ClockTree::Source::PLL3Q && (t.pll3_m == 0 || t.pll3_q == 0))
                compile_error("SPI45 uses PLL3Q but PLL3 not configured");
        }
        if (t.spi6_src != ClockTree::Source::None) {
            if (t.spi6_src == ClockTree::Source::PLL2P && (t.pll2_m == 0 || t.pll2_p == 0))
                compile_error("SPI6 uses PLL2P but PLL2 not configured");
            if (t.spi6_src == ClockTree::Source::PLL3P && (t.pll3_m == 0 || t.pll3_p == 0))
                compile_error("SPI6 uses PLL3P but PLL3 not configured");
        }
        if (t.adc_src == ClockTree::Source::PLL2R && (t.pll2_m == 0 || t.pll2_r == 0))
            compile_error("ADC uses PLL2R but PLL2 not configured");
        if (t.adc_src == ClockTree::Source::PLL3R && (t.pll3_m == 0 || t.pll3_r == 0))
            compile_error("ADC uses PLL3R but PLL3 not configured");
        if (t.sdmmc_src == ClockTree::Source::PLL2R && (t.pll2_m == 0 || t.pll2_r == 0))
            compile_error("SDMMC uses PLL2R but PLL2 not configured");
    }

    template <std::size_t N>
    static consteval void build(std::span<const Entry, N> entries, const ClockTree& tree) {
        validate(tree, entries);
    }

#ifndef SIM_ON

    static constexpr uint32_t to_hal_rge(uint32_t rge) {
        switch (rge) {
        case 0:
            return RCC_PLL1VCIRANGE_0;
        case 1:
            return RCC_PLL1VCIRANGE_1;
        case 2:
            return RCC_PLL1VCIRANGE_2;
        case 3:
            return RCC_PLL1VCIRANGE_3;
        default:
            return RCC_PLL1VCIRANGE_0;
        }
    }

    static constexpr uint32_t to_hal_d1cpre(uint32_t d) {
        switch (d) {
        case 1:
            return RCC_SYSCLK_DIV1;
        case 2:
            return RCC_SYSCLK_DIV2;
        case 4:
            return RCC_SYSCLK_DIV4;
        case 8:
            return RCC_SYSCLK_DIV8;
        case 16:
            return RCC_SYSCLK_DIV16;
        case 64:
            return RCC_SYSCLK_DIV64;
        case 128:
            return RCC_SYSCLK_DIV128;
        case 256:
            return RCC_SYSCLK_DIV256;
        case 512:
            return RCC_SYSCLK_DIV512;
        default:
            return RCC_SYSCLK_DIV1;
        }
    }

    static constexpr uint32_t to_hal_apb_pre(uint32_t d) {
        switch (d) {
        case 1:
            return RCC_APB1_DIV1;
        case 2:
            return RCC_APB1_DIV2;
        case 4:
            return RCC_APB1_DIV4;
        case 8:
            return RCC_APB1_DIV8;
        case 16:
            return RCC_APB1_DIV16;
        default:
            return RCC_APB1_DIV2;
        }
    }

    static constexpr uint32_t to_hal_flash_latency(uint32_t ws) {
        return (ws >= 1 && ws <= 4) ? (FLASH_LATENCY_0 + ws) : FLASH_LATENCY_4;
    }

    static void apply_system_clocks(const ClockTree& t) {
        RCC_OscInitTypeDef osc = {};
        RCC_ClkInitTypeDef clk = {};

        HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
        __HAL_PWR_VOLTAGESCALING_CONFIG(
            sysclk(t) > 480'000'000 ? PWR_REGULATOR_VOLTAGE_SCALE0 : PWR_REGULATOR_VOLTAGE_SCALE1
        );
        while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {
        }

        __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSE);

        osc.OscillatorType =
            RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_HSE;
        osc.HSIState = RCC_HSI_DIV1;
        osc.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
        osc.LSIState = RCC_LSI_ON;
        osc.HSEState = t.hse_bypass ? RCC_HSE_BYPASS : RCC_HSE_ON;

        osc.PLL.PLLState = RCC_PLL_ON;
        osc.PLL.PLLSource = RCC_PLLSOURCE_HSE;
        osc.PLL.PLLM = t.pll1_m;
        osc.PLL.PLLN = t.pll1_n;
        osc.PLL.PLLP = t.pll1_p;
        osc.PLL.PLLQ = t.pll1_q != 0 ? t.pll1_q : 1;
        osc.PLL.PLLR = t.pll1_r != 0 ? t.pll1_r : 1;
        osc.PLL.PLLRGE = to_hal_rge(pll1_rge(t));
        osc.PLL.PLLVCOSEL = pll1_vco_wide(t) ? RCC_PLL1VCOWIDE : RCC_PLL1VCOMEDIUM;
        osc.PLL.PLLFRACN = t.pll1_fracn;

        if (HAL_RCC_OscConfig(&osc) != HAL_OK) {
            PANIC("ClockDomain: HAL_RCC_OscConfig failed");
        }

        clk.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 |
                        RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
        clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
        clk.SYSCLKDivider = to_hal_d1cpre(t.d1cpre);
        clk.AHBCLKDivider = RCC_HCLK_DIV1;
        clk.APB3CLKDivider = RCC_APB3_DIV2;
        clk.APB1CLKDivider = to_hal_apb_pre(t.d2ppre1);
        clk.APB2CLKDivider = to_hal_apb_pre(t.d2ppre2);
        clk.APB4CLKDivider = RCC_APB4_DIV2;

        if (HAL_RCC_ClockConfig(&clk, to_hal_flash_latency(flash_latency(t))) != HAL_OK) {
            PANIC("ClockDomain: HAL_RCC_ClockConfig failed");
        }
    }

    static void apply_peripheral_clocks(const ClockTree& t) {
        RCC_PeriphCLKInitTypeDef pclk = {};

        // PLL2/PLL3 register config — HAL requires RCC_PERIPHCLK_* to enable each PLL block
        if (t.pll2_m != 0) {
            pclk.PeriphClockSelection |= RCC_PERIPHCLK_ADC; // triggers PLL2 register write
            pclk.PLL2.PLL2M = t.pll2_m;
            pclk.PLL2.PLL2N = t.pll2_n;
            pclk.PLL2.PLL2P = t.pll2_p != 0 ? t.pll2_p : 1;
            pclk.PLL2.PLL2Q = t.pll2_q != 0 ? t.pll2_q : 1;
            pclk.PLL2.PLL2R = t.pll2_r != 0 ? t.pll2_r : 1;
            pclk.PLL2.PLL2RGE = to_hal_rge(pll2_rge(t));
            pclk.PLL2.PLL2VCOSEL = RCC_PLL2VCOMEDIUM;
            pclk.PLL2.PLL2FRACN = t.pll2_fracn;
        }
        if (t.pll3_m != 0) {
            pclk.PeriphClockSelection |= RCC_PERIPHCLK_FDCAN; // triggers PLL3 register write
            pclk.PLL3.PLL3M = t.pll3_m;
            pclk.PLL3.PLL3N = t.pll3_n;
            pclk.PLL3.PLL3P = t.pll3_p != 0 ? t.pll3_p : 1;
            pclk.PLL3.PLL3Q = t.pll3_q != 0 ? t.pll3_q : 1;
            pclk.PLL3.PLL3R = t.pll3_r != 0 ? t.pll3_r : 1;
            pclk.PLL3.PLL3RGE = to_hal_rge(pll3_rge(t));
            pclk.PLL3.PLL3VCOSEL = RCC_PLL3VCOMEDIUM;
            pclk.PLL3.PLL3FRACN = t.pll3_fracn;
        }

        pclk.PeriphClockSelection |= RCC_PERIPHCLK_SPI1;
        switch (t.spi123_src) {
        case ClockTree::Source::PLL1Q:
            pclk.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL;
            break;
        case ClockTree::Source::PLL2P:
            pclk.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL2;
            break;
        case ClockTree::Source::PLL3P:
            pclk.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL3;
            break;
        case ClockTree::Source::HSI:
            pclk.Spi123ClockSelection = RCC_SPI123CLKSOURCE_CLKP;
            break;
        default:
            PANIC("ClockDomain: SPI123 clock source not supported");
            break;
        }

        pclk.PeriphClockSelection |= RCC_PERIPHCLK_SPI4;
        switch (t.spi45_src) {
        case ClockTree::Source::PCLK2:
            pclk.Spi45ClockSelection = RCC_SPI45CLKSOURCE_D2PCLK2;
            break;
        case ClockTree::Source::PLL2Q:
            pclk.Spi45ClockSelection = RCC_SPI45CLKSOURCE_PLL2;
            break;
        case ClockTree::Source::PLL3Q:
            pclk.Spi45ClockSelection = RCC_SPI45CLKSOURCE_PLL3;
            break;
        case ClockTree::Source::HSI:
            pclk.Spi45ClockSelection = RCC_SPI45CLKSOURCE_HSI;
            break;
        case ClockTree::Source::CSI:
            pclk.Spi45ClockSelection = RCC_SPI45CLKSOURCE_CSI;
            break;
        case ClockTree::Source::HSE:
            pclk.Spi45ClockSelection = RCC_SPI45CLKSOURCE_HSE;
            break;
        default:
            PANIC("ClockDomain: SPI45 clock source not supported");
            break;
        }

        pclk.PeriphClockSelection |= RCC_PERIPHCLK_SPI6;
        switch (t.spi6_src) {
        case ClockTree::Source::PCLK2:
            pclk.Spi6ClockSelection = RCC_SPI6CLKSOURCE_D3PCLK1;
            break;
        case ClockTree::Source::PLL2P:
            pclk.Spi6ClockSelection = RCC_SPI6CLKSOURCE_PLL2;
            break;
        case ClockTree::Source::PLL3P:
            pclk.Spi6ClockSelection = RCC_SPI6CLKSOURCE_PLL3;
            break;
        case ClockTree::Source::HSI:
            pclk.Spi6ClockSelection = RCC_SPI6CLKSOURCE_HSI;
            break;
        case ClockTree::Source::CSI:
            pclk.Spi6ClockSelection = RCC_SPI6CLKSOURCE_CSI;
            break;
        case ClockTree::Source::HSE:
            pclk.Spi6ClockSelection = RCC_SPI6CLKSOURCE_HSE;
            break;
        default:
            PANIC("ClockDomain: SPI6 clock source not supported");
            break;
        }

        if (t.sdmmc_src != ClockTree::Source::None) {
            pclk.PeriphClockSelection |= RCC_PERIPHCLK_SDMMC;
            switch (t.sdmmc_src) {
            case ClockTree::Source::PLL1Q:
                pclk.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL;
                __HAL_RCC_PLLCLKOUT_ENABLE(RCC_PLL1_DIVQ);
                break;
            case ClockTree::Source::PLL2R:
                pclk.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL2;
                break;
            default:
                PANIC("ClockDomain: SDMMC clock source not supported");
                break;
            }
        }

        // Enable PLL2/PLL3 outputs that peripherals depend on
        if (t.spi123_src == ClockTree::Source::PLL2P || t.spi6_src == ClockTree::Source::PLL2P)
            __HAL_RCC_PLL2CLKOUT_ENABLE(RCC_PLL2_DIVP);
        if (t.spi45_src == ClockTree::Source::PLL2Q || t.fdcan_src == ClockTree::Source::PLL2Q)
            __HAL_RCC_PLL2CLKOUT_ENABLE(RCC_PLL2_DIVQ);
        if (t.adc_src == ClockTree::Source::PLL2R || t.sdmmc_src == ClockTree::Source::PLL2R)
            __HAL_RCC_PLL2CLKOUT_ENABLE(RCC_PLL2_DIVR);

        if (t.spi123_src == ClockTree::Source::PLL3P || t.spi6_src == ClockTree::Source::PLL3P)
            __HAL_RCC_PLL3CLKOUT_ENABLE(RCC_PLL3_DIVP);
        if (t.spi45_src == ClockTree::Source::PLL3Q)
            __HAL_RCC_PLL3CLKOUT_ENABLE(RCC_PLL3_DIVQ);
        if (t.adc_src == ClockTree::Source::PLL3R)
            __HAL_RCC_PLL3CLKOUT_ENABLE(RCC_PLL3_DIVR);

        if (t.adc_src != ClockTree::Source::None) {
            pclk.PeriphClockSelection |= RCC_PERIPHCLK_ADC;
            switch (t.adc_src) {
            case ClockTree::Source::HSE:
                pclk.AdcClockSelection = RCC_ADCCLKSOURCE_CLKP;
                break;
            case ClockTree::Source::PLL2R:
                pclk.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;
                break;
            case ClockTree::Source::PLL3R:
                pclk.AdcClockSelection = RCC_ADCCLKSOURCE_PLL3;
                break;
            default:
                PANIC("ClockDomain: ADC clock source not supported");
                break;
            }
        }

        if (t.fdcan_src != ClockTree::Source::None) {
            pclk.PeriphClockSelection |= RCC_PERIPHCLK_FDCAN;
            switch (t.fdcan_src) {
            case ClockTree::Source::PLL2Q:
                pclk.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL2;
                break;
            case ClockTree::Source::PLL1Q:
                pclk.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL;
                break;
            case ClockTree::Source::HSE:
                pclk.FdcanClockSelection = RCC_FDCANCLKSOURCE_HSE;
                break;
            default:
                PANIC("ClockDomain: FDCAN clock source not supported");
                break;
            }
        }

        if (pclk.PeriphClockSelection != 0) {
            if (HAL_RCCEx_PeriphCLKConfig(&pclk) != HAL_OK) {
                PANIC("ClockDomain: HAL_RCCEx_PeriphCLKConfig failed");
            }
        }
    }

#endif // SIM_ON

    struct Init {
        static inline void init(const ClockTree& tree) {
            s_tree = &tree;
#ifndef SIM_ON
            apply_system_clocks(tree);
            apply_peripheral_clocks(tree);
#endif
        }
    };
};

// ─── Default clock tree ───

#ifndef HSE_VALUE
#define HSE_VALUE 25'000'000
#endif

#if HSE_VALUE == 8'000'000
inline constexpr ClockDomain::ClockTree default_clock_tree{
    .hse_frequency = 8'000'000,
    .hse_bypass = true,
    .pll1_m = 4,
    .pll1_n = 275,
    .pll1_p = 1,
    .pll1_q = 4,
    .pll1_r = 2,
    .d1cpre = 2,
    .spi123_src = ST_LIB::ClockDomain::ClockTree::Source::HSI,
    .spi45_src = ST_LIB::ClockDomain::ClockTree::Source::HSI,
    .spi6_src = ST_LIB::ClockDomain::ClockTree::Source::HSI,
    .adc_src = ST_LIB::ClockDomain::ClockTree::Source::HSI,
    .fdcan_src = ST_LIB::ClockDomain::ClockTree::Source::HSI,
    .sdmmc_src = ST_LIB::ClockDomain::ClockTree::Source::HSI,
};
#elif HSE_VALUE == 25'000'000
inline constexpr ClockDomain::ClockTree default_clock_tree{
    .hse_frequency = 25'000'000,
    .hse_bypass = false,
    .pll1_m = 5,
    .pll1_n = 110,
    .pll1_p = 1,
    .pll1_q = 4,
    .pll1_r = 2,
    .pll2_m = 5,
    .pll2_n = 160,
    .pll2_p = 2,
    .pll2_q = 4,
    .pll2_r = 2,
    .d1cpre = 2,
    .spi123_src = ST_LIB::ClockDomain::ClockTree::Source::PLL1Q,
    .spi45_src = ST_LIB::ClockDomain::ClockTree::Source::PLL2Q,
    .spi6_src = ST_LIB::ClockDomain::ClockTree::Source::PLL2P,
    .adc_src = ST_LIB::ClockDomain::ClockTree::Source::PLL2R,
    .fdcan_src = ST_LIB::ClockDomain::ClockTree::Source::HSE,
    .sdmmc_src = ST_LIB::ClockDomain::ClockTree::Source::PLL1Q,
};
#else
inline constexpr ClockDomain::ClockTree default_clock_tree{};
#endif

} // namespace ST_LIB
