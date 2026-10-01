#pragma once

#include "hal_wrapper.h"

#include <algorithm>
#include <array>
#include <span>
#include <utility>

#include "ErrorHandler/ErrorHandler.hpp"
#include "HALAL/Models/DMA/DMA2.hpp"
#include "HALAL/Models/Clocks/ClockDomain.hpp"
#include "HALAL/Models/GPIO.hpp"

#include "HALAL/Models/Pin.hpp"
#ifdef SIM_ON
#define STLIB_ADC_DMA_BUFFER_ATTR
#else
#include "HALAL/Models/MPU.hpp"
#define STLIB_ADC_DMA_BUFFER_ATTR D1_NC
#endif

using std::array;
using std::size_t;
using std::span;

#ifdef HAL_ADC_MODULE_ENABLED
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern ADC_HandleTypeDef hadc3;
#endif

namespace ST_LIB {
extern void compile_error(const char* msg);

#ifdef HAL_ADC_MODULE_ENABLED
using ::hadc1;
using ::hadc2;
using ::hadc3;

struct ADCDomain {
    static constexpr std::size_t max_channels_per_peripheral = 16;
    enum class Peripheral : uint8_t { AUTO, ADC_1, ADC_2, ADC_3 };

    enum class Resolution : uint32_t {
        BITS_16 = ADC_RESOLUTION_16B,
        BITS_14 = ADC_RESOLUTION_14B,
        BITS_12 = ADC_RESOLUTION_12B,
        BITS_10 = ADC_RESOLUTION_10B,
        BITS_8 = ADC_RESOLUTION_8B,
    };

    enum class SampleTime : uint32_t {
        CYCLES_1_5 = ADC_SAMPLETIME_1CYCLE_5,
        CYCLES_2_5 = ADC_SAMPLETIME_2CYCLES_5,
        CYCLES_8_5 = ADC_SAMPLETIME_8CYCLES_5,
        CYCLES_16_5 = ADC_SAMPLETIME_16CYCLES_5,
        CYCLES_32_5 = ADC_SAMPLETIME_32CYCLES_5,
        CYCLES_64_5 = ADC_SAMPLETIME_64CYCLES_5,
        CYCLES_387_5 = ADC_SAMPLETIME_387CYCLES_5,
        CYCLES_810_5 = ADC_SAMPLETIME_810CYCLES_5,
    };

    enum class Channel : uint32_t {
        AUTO = 0xFFFFFFFFu,
        CH0 = ADC_CHANNEL_0,
        CH1 = ADC_CHANNEL_1,
        CH2 = ADC_CHANNEL_2,
        CH3 = ADC_CHANNEL_3,
        CH4 = ADC_CHANNEL_4,
        CH5 = ADC_CHANNEL_5,
        CH6 = ADC_CHANNEL_6,
        CH7 = ADC_CHANNEL_7,
        CH8 = ADC_CHANNEL_8,
        CH9 = ADC_CHANNEL_9,
        CH10 = ADC_CHANNEL_10,
        CH11 = ADC_CHANNEL_11,
        CH12 = ADC_CHANNEL_12,
        CH13 = ADC_CHANNEL_13,
        CH14 = ADC_CHANNEL_14,
        CH15 = ADC_CHANNEL_15,
        CH16 = ADC_CHANNEL_16,
        CH17 = ADC_CHANNEL_17,
        CH18 = ADC_CHANNEL_18,
        CH19 = ADC_CHANNEL_19,
        VREFINT = ADC_CHANNEL_VREFINT,
        TEMPSENSOR = ADC_CHANNEL_TEMPSENSOR,
        VBAT = ADC_CHANNEL_VBAT,
    };

    struct Entry {
        size_t gpio_idx;
        GPIODomain::Pin pin;
        Peripheral peripheral;
        Channel channel;
        Resolution resolution;
        SampleTime sample_time;

        uint32_t sample_rate_hz;
        float* output;
    };

    template <uint32_t MaxADCCLK> struct ADCClockModel {
        static constexpr auto group = ClockDomain::ClockGroup::ADC_G;

        static constexpr uint32_t prescalers[] = {1, 2, 4, 6, 8, 10, 12, 16, 32, 64, 128, 256};
        static constexpr uint32_t ADC_CLK_MIN = 500'000;

        static constexpr bool try_solve(uint32_t kernel_clk) {
            for (uint32_t p : prescalers) {
                uint32_t adc_clk = kernel_clk / p;
                if (adc_clk >= ADC_CLK_MIN && adc_clk <= MaxADCCLK)
                    return true;
            }
            return false;
        }
    };

    static constexpr uint32_t adc_max_clk(Resolution res) {
        switch (res) {
        case Resolution::BITS_16:
            return 8'333'333;
        case Resolution::BITS_14:
            return 16'666'666;
        case Resolution::BITS_12:
            return 36'000'000;
        case Resolution::BITS_10:
            return 50'000'000;
        case Resolution::BITS_8:
            return 50'000'000;
        }
        return 36'000'000;
    }

    static constexpr uint32_t prescaler_to_hal(uint32_t p) {
        switch (p) {
        case 1:
            return ADC_CLOCK_ASYNC_DIV1;
        case 2:
            return ADC_CLOCK_ASYNC_DIV2;
        case 4:
            return ADC_CLOCK_ASYNC_DIV4;
        case 6:
            return ADC_CLOCK_ASYNC_DIV6;
        case 8:
            return ADC_CLOCK_ASYNC_DIV8;
        case 10:
            return ADC_CLOCK_ASYNC_DIV10;
        case 12:
            return ADC_CLOCK_ASYNC_DIV12;
        case 16:
            return ADC_CLOCK_ASYNC_DIV16;
        case 32:
            return ADC_CLOCK_ASYNC_DIV32;
        case 64:
            return ADC_CLOCK_ASYNC_DIV64;
        case 128:
            return ADC_CLOCK_ASYNC_DIV128;
        case 256:
            return ADC_CLOCK_ASYNC_DIV256;
        default:
            return ADC_CLOCK_ASYNC_DIV4;
        }
    }

    template <Resolution Res = Resolution::BITS_12> struct ADC {
        GPIODomain::GPIO gpio;
        using domain = ADCDomain;
        using Model = ADCClockModel<adc_max_clk(Res)>;

        static constexpr auto resolution = Res;
        Entry e;
        ClockDomain::Device clock_device = {
            .group = Model::group,
            .try_solve = &Model::try_solve,
        };

        consteval ADC(
            const GPIODomain::Pin& pin,
            float& output,
            SampleTime sample_time = SampleTime::CYCLES_8_5,
            uint32_t sample_rate_hz = 0,
            Peripheral peripheral = Peripheral::AUTO,
            Channel channel = Channel::AUTO
        )
            : gpio{pin, GPIODomain::OperationMode::ANALOG, GPIODomain::Pull::None, GPIODomain::Speed::Low},
              e{.gpio_idx = 0,
                .pin = pin,
                .peripheral = peripheral,
                .channel = channel,
                .resolution = Res,
                .sample_time = sample_time,
                .sample_rate_hz = sample_rate_hz,
                .output = &output} {}

        consteval ADC(
            const GPIODomain::Pin& pin,
            SampleTime sample_time = SampleTime::CYCLES_8_5,
            uint32_t sample_rate_hz = 0,
            Peripheral peripheral = Peripheral::AUTO,
            Channel channel = Channel::AUTO
        )
            : gpio{pin, GPIODomain::OperationMode::ANALOG, GPIODomain::Pull::None, GPIODomain::Speed::Low},
              e{.gpio_idx = 0,
                .pin = pin,
                .peripheral = peripheral,
                .channel = channel,
                .resolution = Res,
                .sample_time = sample_time,
                .sample_rate_hz = sample_rate_hz,
                .output = nullptr} {}

        consteval ADC(
            const GPIODomain::Pin& pin,
            Peripheral peripheral,
            Channel channel,
            float& output,
            SampleTime sample_time = SampleTime::CYCLES_8_5,
            uint32_t sample_rate_hz = 0
        )
            : ADC(pin, output, sample_time, sample_rate_hz, peripheral, channel) {}

        consteval ADC(
            const GPIODomain::Pin& pin,
            Peripheral peripheral,
            Channel channel,
            SampleTime sample_time = SampleTime::CYCLES_8_5,
            uint32_t sample_rate_hz = 0
        )
            : ADC(pin, sample_time, sample_rate_hz, peripheral, channel) {}

        template <class Ctx> consteval std::size_t inscribe(Ctx& ctx) const {
            const auto gpio_idx = gpio.inscribe(ctx);
            Entry entry = e;
            entry.gpio_idx = gpio_idx;
            const auto resolved = resolve_mapping(entry);
            entry.peripheral = resolved.first;
            entry.channel = resolved.second;

            // Register clock requirement with ClockDomain
            clock_device.inscribe(ctx);

            return ctx.template add<ADCDomain>(entry, this);
        }
    };

    static constexpr std::size_t max_instances{32};

    struct Config {
        size_t gpio_idx;
        Peripheral peripheral;
        Channel channel;
        Resolution resolution;
        SampleTime sample_time;

        uint32_t sample_rate_hz;
        uint32_t dma_request;
        float* output;
    };

    static constexpr uint8_t peripheral_index(Peripheral p) {
        switch (p) {
        case Peripheral::AUTO:
            return 0;
        case Peripheral::ADC_1:
            return 0;
        case Peripheral::ADC_2:
            return 1;
        case Peripheral::ADC_3:
            return 2;
        }
        return 0;
    }

    static constexpr Peripheral peripheral_from_index(uint8_t idx) {
        switch (idx) {
        case 0:
            return Peripheral::ADC_1;
        case 1:
            return Peripheral::ADC_2;
        case 2:
            return Peripheral::ADC_3;
        default:
            return Peripheral::ADC_1;
        }
    }

    static consteval DMADomain::Peripheral dma_peripheral(Peripheral p) {
        switch (p) {
        case Peripheral::ADC_1:
            return DMADomain::Peripheral::adc1;
        case Peripheral::ADC_2:
            return DMADomain::Peripheral::adc2;
        case Peripheral::ADC_3:
            return DMADomain::Peripheral::adc3;
        case Peripheral::AUTO:
            break;
        }
        return DMADomain::Peripheral::adc1;
    }

    static consteval uint32_t dma_request(Peripheral p) {
        return DMADomain::get_Request(dma_peripheral(p), 0);
    }

    static constexpr std::array<uint32_t, max_channels_per_peripheral> regular_ranks{{
        ADC_REGULAR_RANK_1,
        ADC_REGULAR_RANK_2,
        ADC_REGULAR_RANK_3,
        ADC_REGULAR_RANK_4,
        ADC_REGULAR_RANK_5,
        ADC_REGULAR_RANK_6,
        ADC_REGULAR_RANK_7,
        ADC_REGULAR_RANK_8,
        ADC_REGULAR_RANK_9,
        ADC_REGULAR_RANK_10,
        ADC_REGULAR_RANK_11,
        ADC_REGULAR_RANK_12,
        ADC_REGULAR_RANK_13,
        ADC_REGULAR_RANK_14,
        ADC_REGULAR_RANK_15,
        ADC_REGULAR_RANK_16,
    }};

    static constexpr uint32_t regular_rank(uint8_t zero_based_rank) {
        if (zero_based_rank >= regular_ranks.size()) {
            return ADC_REGULAR_RANK_1;
        }
        return regular_ranks[zero_based_rank];
    }

    struct BufferSizes {
        std::size_t adc1_size = 0;
        std::size_t adc2_size = 0;
        std::size_t adc3_size = 0;
    };

    static consteval BufferSizes calculate_buffer_sizes(span<const Config> cfgs) {
        BufferSizes sizes;
        for (const auto& cfg : cfgs) {
            switch (cfg.peripheral) {
            case Peripheral::ADC_1:
                sizes.adc1_size++;
                break;
            case Peripheral::ADC_2:
                sizes.adc2_size++;
                break;
            case Peripheral::ADC_3:
                sizes.adc3_size++;
                break;
            case Peripheral::AUTO:
                compile_error("ADC: peripheral not resolved");
                break;
            }
        }
        return sizes;
    }

    static consteval bool is_valid_channel(Channel ch) {
        switch (ch) {
        case Channel::AUTO:
            return true;
        case Channel::CH0:
        case Channel::CH1:
        case Channel::CH2:
        case Channel::CH3:
        case Channel::CH4:
        case Channel::CH5:
        case Channel::CH6:
        case Channel::CH7:
        case Channel::CH8:
        case Channel::CH9:
        case Channel::CH10:
        case Channel::CH11:
        case Channel::CH12:
        case Channel::CH13:
        case Channel::CH14:
        case Channel::CH15:
        case Channel::CH16:
        case Channel::CH17:
        case Channel::CH18:
        case Channel::CH19:
        case Channel::VREFINT:
        case Channel::TEMPSENSOR:
        case Channel::VBAT:
            return true;
        }
        return false;
    }

    static consteval uint8_t resolution_bits(Resolution r) {
        switch (r) {
        case Resolution::BITS_16:
            return 16;
        case Resolution::BITS_14:
            return 14;
        case Resolution::BITS_12:
            return 12;
        case Resolution::BITS_10:
            return 10;
        case Resolution::BITS_8:
            return 8;
        }
        return 12;
    }

    static consteval bool is_internal_channel(Channel ch) {
        switch (ch) {
        case Channel::VREFINT:
        case Channel::TEMPSENSOR:
        case Channel::VBAT:
            return true;
        default:
            return false;
        }
    }

    static consteval bool internal_channel_allowed(Peripheral p, Channel ch) {
        if (!is_internal_channel(ch)) {
            return true;
        }
#if STLIB_HAS_ADC3
        return p == Peripheral::ADC_3;
#else
        return p == Peripheral::ADC_2;
#endif
    }

    static consteval bool resolution_supported(Peripheral p, Resolution r) {
        if (p == Peripheral::ADC_3) {
            return resolution_bits(r) <= 12;
        }
        return true;
    }

    struct PinMapping {
        GPIODomain::Pin pin;
        Peripheral peripheral;
        Channel channel;
        uint8_t max_bits;
    };

    static constexpr uint8_t max_bits_adc12 = 16;
    static constexpr uint8_t max_bits_adc3 = 12;

    static constexpr std::array<PinMapping, 42> pin_map{{
        {PA0, Peripheral::ADC_1, Channel::CH16, max_bits_adc12},
        {PA1, Peripheral::ADC_1, Channel::CH17, max_bits_adc12},
        {PA2, Peripheral::ADC_1, Channel::CH14, max_bits_adc12},
        {PA3, Peripheral::ADC_1, Channel::CH15, max_bits_adc12},
        {PA4, Peripheral::ADC_1, Channel::CH18, max_bits_adc12},
        {PA5, Peripheral::ADC_1, Channel::CH19, max_bits_adc12},
        {PA6, Peripheral::ADC_1, Channel::CH3, max_bits_adc12},
        {PA7, Peripheral::ADC_1, Channel::CH7, max_bits_adc12},
        {PB0, Peripheral::ADC_1, Channel::CH9, max_bits_adc12},
        {PB1, Peripheral::ADC_1, Channel::CH5, max_bits_adc12},
        {PC0, Peripheral::ADC_1, Channel::CH10, max_bits_adc12},
        {PC1, Peripheral::ADC_1, Channel::CH11, max_bits_adc12},
        {PC4, Peripheral::ADC_1, Channel::CH4, max_bits_adc12},
        {PC5, Peripheral::ADC_1, Channel::CH8, max_bits_adc12},
        {PF11, Peripheral::ADC_1, Channel::CH2, max_bits_adc12},
        {PF12, Peripheral::ADC_1, Channel::CH6, max_bits_adc12},

        {PA2, Peripheral::ADC_2, Channel::CH14, max_bits_adc12},
        {PA3, Peripheral::ADC_2, Channel::CH15, max_bits_adc12},
        {PA4, Peripheral::ADC_2, Channel::CH18, max_bits_adc12},
        {PA5, Peripheral::ADC_2, Channel::CH19, max_bits_adc12},
        {PA6, Peripheral::ADC_2, Channel::CH3, max_bits_adc12},
        {PA7, Peripheral::ADC_2, Channel::CH7, max_bits_adc12},
        {PB0, Peripheral::ADC_2, Channel::CH9, max_bits_adc12},
        {PB1, Peripheral::ADC_2, Channel::CH5, max_bits_adc12},
        {PC0, Peripheral::ADC_2, Channel::CH10, max_bits_adc12},
        {PC1, Peripheral::ADC_2, Channel::CH11, max_bits_adc12},
        {PC4, Peripheral::ADC_2, Channel::CH4, max_bits_adc12},
        {PC5, Peripheral::ADC_2, Channel::CH8, max_bits_adc12},
        {PF13, Peripheral::ADC_2, Channel::CH2, max_bits_adc12},
        {PF14, Peripheral::ADC_2, Channel::CH6, max_bits_adc12},

        {PC0, Peripheral::ADC_3, Channel::CH10, max_bits_adc3},
        {PC1, Peripheral::ADC_3, Channel::CH11, max_bits_adc3},
        {PC2, Peripheral::ADC_3, Channel::CH0, max_bits_adc3},
        {PC3, Peripheral::ADC_3, Channel::CH1, max_bits_adc3},
        {PF3, Peripheral::ADC_3, Channel::CH5, max_bits_adc3},
        {PF4, Peripheral::ADC_3, Channel::CH9, max_bits_adc3},
        {PF5, Peripheral::ADC_3, Channel::CH4, max_bits_adc3},
        {PF6, Peripheral::ADC_3, Channel::CH8, max_bits_adc3},
        {PF7, Peripheral::ADC_3, Channel::CH3, max_bits_adc3},
        {PF8, Peripheral::ADC_3, Channel::CH7, max_bits_adc3},
        {PF9, Peripheral::ADC_3, Channel::CH2, max_bits_adc3},
        {PF10, Peripheral::ADC_3, Channel::CH6, max_bits_adc3},
    }};

    static consteval uint8_t preference_score(Resolution r, Peripheral p) {
        const uint8_t bits = resolution_bits(r);
        if (bits > 12) {
            switch (p) {
            case Peripheral::ADC_1:
                return 3;
            case Peripheral::ADC_2:
                return 2;
            case Peripheral::ADC_3:
                return 0;
            case Peripheral::AUTO:
                return 0;
            }
        } else {
            switch (p) {
            case Peripheral::ADC_3:
                return 3;
            case Peripheral::ADC_1:
                return 2;
            case Peripheral::ADC_2:
                return 1;
            case Peripheral::AUTO:
                return 0;
            }
        }
        return 0;
    }

    static consteval std::pair<Peripheral, Channel> resolve_mapping(const Entry& e) {
        if (e.peripheral != Peripheral::AUTO && e.channel != Channel::AUTO) {
            if (!is_valid_channel(e.channel)) {
                compile_error("ADC: invalid channel");
            }
            if (!internal_channel_allowed(e.peripheral, e.channel)) {
                compile_error("ADC: internal channel must use dedicated ADC");
            }
            if (!resolution_supported(e.peripheral, e.resolution)) {
                compile_error("ADC: resolution not supported by selected ADC");
            }
            return {e.peripheral, e.channel};
        }

        if (e.channel != Channel::AUTO && is_internal_channel(e.channel)) {
            const Peripheral p = (e.peripheral == Peripheral::AUTO)
#if STLIB_HAS_ADC3
                                     ? Peripheral::ADC_3
#else
                                     ? Peripheral::ADC_2
#endif
                                     : e.peripheral;

            if (!internal_channel_allowed(p, e.channel)) {
                compile_error("ADC: internal channel must use dedicated ADC");
            }
            if (!resolution_supported(p, e.resolution)) {
                compile_error("ADC: resolution not supported by selected ADC");
            }
            return {p, e.channel};
        }

        bool found = false;
        Peripheral best_peripheral = Peripheral::AUTO;
        Channel best_channel = Channel::AUTO;
        uint8_t best_score = 0;

        for (const auto& m : pin_map) {
            if (m.pin != e.pin) {
                continue;
            }
            if (e.peripheral != Peripheral::AUTO && m.peripheral != e.peripheral) {
                continue;
            }
            if (e.channel != Channel::AUTO && m.channel != e.channel) {
                continue;
            }
            if (resolution_bits(e.resolution) > m.max_bits) {
                continue;
            }

            const uint8_t score = preference_score(e.resolution, m.peripheral);
            if (!found || score > best_score) {
                found = true;
                best_score = score;
                best_peripheral = m.peripheral;
                best_channel = m.channel;
            }
        }

        if (!found) {
            compile_error("ADC: no valid ADC mapping for pin/resolution");
        }

        return {best_peripheral, best_channel};
    }

    template <size_t N> static consteval array<Config, N> build(span<const Entry> entries) {
        static_assert(N <= max_instances, "ADCDomain: too many instances");
        if (entries.size() != N) {
            compile_error("ADC: build entries size mismatch");
        }

        array<Config, N> cfgs{};
        array<bool, 3> periph_seen{};
        array<Resolution, 3> periph_resolution{};
        array<uint32_t, 3> periph_rate{};
        array<uint8_t, 3> periph_counts{};

        for (std::size_t i = 0; i < N; ++i) {
            const auto& e = entries[i];
            const auto [peripheral, channel] = resolve_mapping(e);

            if (!is_valid_channel(channel)) {
                compile_error("ADC: invalid channel");
            }
            if (!internal_channel_allowed(peripheral, channel)) {
                compile_error("ADC: internal channel must use dedicated ADC");
            }
            if (!resolution_supported(peripheral, e.resolution)) {
                compile_error("ADC: resolution not supported by selected ADC");
            }
            if (e.sample_rate_hz != 0) {
                compile_error("ADC: sample_rate_hz is not supported in DMA mode");
            }

            const auto pidx = peripheral_index(peripheral);

            if (!periph_seen[pidx]) {
                periph_seen[pidx] = true;
                periph_resolution[pidx] = e.resolution;
                periph_rate[pidx] = e.sample_rate_hz;
            } else {
                if (periph_resolution[pidx] != e.resolution) {
                    compile_error("ADC: resolution mismatch on same peripheral");
                }
                if (periph_rate[pidx] != e.sample_rate_hz) {
                    compile_error("ADC: sample rate mismatch on same peripheral");
                }
            }

            ++periph_counts[pidx];
            if (periph_counts[pidx] > max_channels_per_peripheral) {
                compile_error("ADC: too many channels on same peripheral");
            }

            for (std::size_t j = 0; j < i; ++j) {
                const auto& prev = entries[j];
                if (prev.gpio_idx == e.gpio_idx) {
                    compile_error("ADC: GPIO already used");
                }
                const auto [prev_peripheral, prev_channel] = resolve_mapping(prev);
                if (prev_peripheral == peripheral && prev_channel == channel) {
                    compile_error("ADC: duplicate channel on same peripheral");
                }
            }

            cfgs[i] = {
                .gpio_idx = e.gpio_idx,
                .peripheral = peripheral,
                .channel = channel,
                .resolution = e.resolution,
                .sample_time = e.sample_time,
                .sample_rate_hz = e.sample_rate_hz,
                .dma_request = dma_request(peripheral),
                .output = e.output,
            };
        }

        return cfgs;
    }

    static consteval bool uses_peripheral(Peripheral peripheral, span<const Config> cfgs) {
        for (const auto& cfg : cfgs) {
            if (cfg.peripheral == peripheral) {
                return true;
            }
        }
        return false;
    }

    static consteval std::size_t
    dma_entries_for_peripheral(Peripheral peripheral, span<const DMADomain::Entry> dma_entries) {
        std::size_t count = 0;
        for (const auto& entry : dma_entries) {
            if (entry.instance != dma_peripheral(peripheral)) {
                continue;
            }
            if (entry.id != 0U) {
                compile_error("ADC: DMA for ADC peripherals must use stream id 0");
            }
            ++count;
        }
        return count;
    }

    static consteval std::size_t
    dma_contribution_count(span<const Config> cfgs, span<const DMADomain::Entry> dma_entries) {
        std::size_t count = 0;
        for (uint8_t pidx = 0; pidx < 3U; ++pidx) {
            const Peripheral peripheral = peripheral_from_index(pidx);
            if (!uses_peripheral(peripheral, cfgs)) {
                continue;
            }

            const auto existing = dma_entries_for_peripheral(peripheral, dma_entries);
            if (existing > 1U) {
                compile_error("ADC: multiple DMA streams configured for the same ADC peripheral");
            }
            if (existing == 0U) {
                ++count;
            }
        }
        return count;
    }

    template <std::size_t ExtraN>
    static consteval std::array<DMADomain::Entry, ExtraN>
    build_dma_contributions(span<const DMADomain::Entry> dma_entries, span<const Config> cfgs) {
        std::array<DMADomain::Entry, ExtraN> extra{};
        std::size_t cursor = 0;

        for (uint8_t pidx = 0; pidx < 3U; ++pidx) {
            const Peripheral peripheral = peripheral_from_index(pidx);
            if (!uses_peripheral(peripheral, cfgs)) {
                continue;
            }

            const auto existing = dma_entries_for_peripheral(peripheral, dma_entries);
            if (existing == 0U) {
                extra[cursor++] = {
                    .instance = dma_peripheral(peripheral),
                    .stream = DMADomain::Stream::none,
                    .irqn = static_cast<IRQn_Type>(0),
                    .id = 0,
                };
            }
        }

        if (cursor != ExtraN) {
            compile_error("ADC: DMA contribution size mismatch");
        }

        return extra;
    }

    static DMADomain::Instance*
    find_dma_instance(uint32_t request, std::span<DMADomain::Instance> dma_peripherals) {
        for (auto& dma_instance : dma_peripherals) {
            if (dma_instance.dma.Init.Request == request) {
                return &dma_instance;
            }
        }
        return nullptr;
    }

    struct Instance {
        ADC_HandleTypeDef* handle = nullptr;
        Channel channel = Channel::CH0;
        SampleTime sample_time = SampleTime::CYCLES_8_5;
        Resolution resolution = Resolution::BITS_12;
        float* output = nullptr;
        volatile uint16_t* dma_slot = nullptr;

        static constexpr uint32_t max_raw_for_resolution(Resolution r) {
            switch (r) {
            case Resolution::BITS_16:
                return 65535U;
            case Resolution::BITS_14:
                return 16383U;
            case Resolution::BITS_12:
                return 4095U;
            case Resolution::BITS_10:
                return 1023U;
            case Resolution::BITS_8:
                return 255U;
            }
            return 4095U;
        }

    private:
        uint32_t sample_raw() const {
            if (dma_slot == nullptr) {
                return 0;
            }
            const uint32_t raw = *dma_slot;
            return std::min<uint32_t>(raw, max_raw_for_resolution(resolution));
        }

    public:
        float get_raw(uint32_t timeout_ms = 2) {
            (void)timeout_ms;
            return static_cast<float>(sample_raw());
        }

        float get_value_from_raw(float raw, float vref = 3.3f) const {
            const float max_val = static_cast<float>(max_raw_for_resolution(resolution));
            if (max_val <= 0.0f) {
                return 0.0f;
            }
            return (raw / max_val) * vref;
        }

        float get_value(uint32_t timeout_ms = 2, float vref = 3.3f) {
            (void)timeout_ms;
            const float raw = get_raw(timeout_ms);
            return get_value_from_raw(raw, vref);
        }

        void read(float vref = 3.3f, uint32_t timeout_ms = 2) {
            if (output == nullptr) {
                return;
            }
            *output = get_value(timeout_ms, vref);
        }
    };

    template <std::size_t N, std::array<Config, N> cfgs> struct Init {
        static inline std::array<Instance, N> instances{};

        static constexpr auto buffer_sizes = calculate_buffer_sizes(cfgs);
        static constexpr std::size_t total_dma_slots =
            buffer_sizes.adc1_size + buffer_sizes.adc2_size + buffer_sizes.adc3_size;
        static_assert(
            total_dma_slots <= max_instances,
            "ADC DMA buffer size exceeds max_instances"
        );

        alignas(32) STLIB_ADC_DMA_BUFFER_ATTR
            static inline uint16_t dma_buffer_pool[total_dma_slots > 0 ? total_dma_slots : 1]{};

        static constexpr bool is_resolved_config(const Config& cfg) {
            return cfg.peripheral != Peripheral::AUTO && cfg.channel != Channel::AUTO;
        }

        static ADC_HandleTypeDef* handle_for(Peripheral p) {
            switch (p) {
            case Peripheral::ADC_1:
                return &hadc1;
            case Peripheral::ADC_2:
                return &hadc2;
            case Peripheral::ADC_3:
                return &hadc3;
            case Peripheral::AUTO:
                break;
            }
            return &hadc1;
        }

        static constexpr std::size_t buffer_size_for(Peripheral peripheral) {
            switch (peripheral) {
            case Peripheral::ADC_1:
                return buffer_sizes.adc1_size;
            case Peripheral::ADC_2:
                return buffer_sizes.adc2_size;
            case Peripheral::ADC_3:
                return buffer_sizes.adc3_size;
            case Peripheral::AUTO:
                break;
            }
            return 0;
        }

        static constexpr std::size_t buffer_offset_for(Peripheral peripheral) {
            switch (peripheral) {
            case Peripheral::ADC_1:
                return 0;
            case Peripheral::ADC_2:
                return buffer_sizes.adc1_size;
            case Peripheral::ADC_3:
                return buffer_sizes.adc1_size + buffer_sizes.adc2_size;
            case Peripheral::AUTO:
                break;
            }
            return 0;
        }
        static volatile uint16_t* get_dma_buffer(Peripheral peripheral) {
            const auto buffer_size = buffer_size_for(peripheral);
            const auto buffer_offset = buffer_offset_for(peripheral);
            if (buffer_size == 0U) {
                PANIC("ADC DMA buffer not available");
                return nullptr;
            }
            if ((buffer_offset + buffer_size) > total_dma_slots) {
                PANIC("ADC DMA pool overflow");
                return nullptr;
            }

            volatile uint16_t* buffer = &dma_buffer_pool[buffer_offset];
            std::fill_n(buffer, buffer_size, uint16_t{0});
            return buffer;
        }

        static volatile uint16_t* get_dma_slot(Peripheral peripheral, uint8_t index) {
            if (index >= buffer_size_for(peripheral)) {
                return nullptr;
            }
            const auto buffer_offset = buffer_offset_for(peripheral);
            if ((buffer_offset + index) >= total_dma_slots) {
                return nullptr;
            }
            return &dma_buffer_pool[buffer_offset + index];
        }

        static void configure_peripheral(const Config& cfg, uint8_t channel_count) {
            ADC_HandleTypeDef* hadc = handle_for(cfg.peripheral);

            if (cfg.peripheral == Peripheral::ADC_1 || cfg.peripheral == Peripheral::ADC_2) {
                __HAL_RCC_ADC12_CLK_ENABLE();
                hadc->Instance = (cfg.peripheral == Peripheral::ADC_1) ? ADC1 : ADC2;
            } else {
                __HAL_RCC_ADC3_CLK_ENABLE();
                hadc->Instance = ADC3;
                HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PC2, SYSCFG_SWITCH_PC2_OPEN);
                HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PC3, SYSCFG_SWITCH_PC3_OPEN);
            }

            uint32_t kernel_clk = ClockDomain::get_kernel_clock(ClockDomain::ClockGroup::ADC_G);
            uint32_t max_clk = adc_max_clk(cfg.resolution);
            uint32_t prescaler = 4;
            constexpr uint32_t prescalers[] = {1, 2, 4, 6, 8, 10, 12, 16, 32, 64, 128, 256};
            for (uint32_t p : prescalers) {
                uint32_t adc_clk = kernel_clk / p;
                if (adc_clk >= 500'000 && adc_clk <= max_clk) {
                    prescaler = p;
                    break;
                }
            }
            hadc->Init.ClockPrescaler = prescaler_to_hal(prescaler);
            hadc->Init.Resolution = static_cast<uint32_t>(cfg.resolution);
            hadc->Init.ScanConvMode = (channel_count > 1U) ? ADC_SCAN_ENABLE : ADC_SCAN_DISABLE;
            hadc->Init.EOCSelection = (channel_count > 1U) ? ADC_EOC_SEQ_CONV : ADC_EOC_SINGLE_CONV;
            hadc->Init.LowPowerAutoWait = DISABLE;
            hadc->Init.ContinuousConvMode = ENABLE;
            hadc->Init.NbrOfConversion = channel_count;
            hadc->Init.DiscontinuousConvMode = DISABLE;
            hadc->Init.NbrOfDiscConversion = 0;
            hadc->Init.ExternalTrigConv = ADC_SOFTWARE_START;
            hadc->Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
            hadc->Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_CIRCULAR;
#if defined(ADC_VER_V5_V90)
            hadc->Init.SamplingMode = ADC_SAMPLING_MODE_NORMAL;
            hadc->Init.DMAContinuousRequests = ENABLE;
#endif
            hadc->Init.Overrun = ADC_OVR_DATA_PRESERVED;
            hadc->Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
            hadc->Init.OversamplingMode = DISABLE;
        }

        static void init(
            std::span<const Config, N> runtime_cfgs,
            std::span<GPIODomain::Instance> gpio_instances = std::span<GPIODomain::Instance>{},
            std::span<DMADomain::Instance> dma_peripherals = std::span<DMADomain::Instance>{}
        ) {
            std::array<bool, 3> periph_ready{false, false, false};
            std::array<uint8_t, 3> periph_channel_counts{0, 0, 0};
            std::array<bool, N> instance_cfg_valid{};
            std::array<uint8_t, N> instance_ranks{};
            (void)gpio_instances;

            for (auto& instance : instances) {
                instance = {};
            }

            for (std::size_t i = 0; i < N; ++i) {
                const auto& cfg = runtime_cfgs[i];
                if (!is_resolved_config(cfg)) {
                    PANIC("ADC config unresolved (AUTO)");
                    continue;
                }
                instance_cfg_valid[i] = true;
                instance_ranks[i] = periph_channel_counts[peripheral_index(cfg.peripheral)]++;
            }

            for (uint8_t pidx = 0; pidx < 3U; ++pidx) {
                const auto channel_count = periph_channel_counts[pidx];
                if (channel_count == 0U) {
                    continue;
                }

                const Peripheral peripheral = peripheral_from_index(pidx);
                const Config* first_cfg = nullptr;
                for (const auto& cfg : runtime_cfgs) {
                    if (cfg.peripheral == peripheral) {
                        first_cfg = &cfg;
                        break;
                    }
                }

                if (first_cfg == nullptr) {
                    continue;
                }

                ADC_HandleTypeDef* hadc = handle_for(peripheral);
                if (hadc->Instance != nullptr && hadc->DMA_Handle != nullptr) {
                    (void)HAL_ADC_Stop_DMA(hadc);
                }

                DMADomain::Instance* dma_instance =
                    find_dma_instance(first_cfg->dma_request, dma_peripherals);
                if (dma_instance == nullptr) {
                    PANIC("ADC DMA instance unavailable");
                    continue;
                }
                volatile uint16_t* buffer = get_dma_buffer(peripheral);
                if (buffer == nullptr) {
                    continue;
                }

                hadc->DMA_Handle = &dma_instance->dma;
                dma_instance->dma.Parent = hadc;
                configure_peripheral(*first_cfg, channel_count);

                if (HAL_ADC_Init(hadc) != HAL_OK) {
                    PANIC("ADC Init failed");
                    continue;
                }

                uint8_t rank = 0;
                bool config_error = false;
                for (const auto& cfg : runtime_cfgs) {
                    if (cfg.peripheral != peripheral) {
                        continue;
                    }

                    ADC_ChannelConfTypeDef sConfig{};
                    sConfig.Channel = static_cast<uint32_t>(cfg.channel);
                    sConfig.Rank = regular_rank(rank);
                    sConfig.SamplingTime = static_cast<uint32_t>(cfg.sample_time);
                    sConfig.SingleDiff = ADC_SINGLE_ENDED;
                    sConfig.OffsetNumber = ADC_OFFSET_NONE;
                    sConfig.Offset = 0;
#if defined(ADC_VER_V5_V90)
                    sConfig.OffsetSignedSaturation = DISABLE;
#endif

                    if (HAL_ADC_ConfigChannel(hadc, &sConfig) != HAL_OK) {
                        PANIC("ADC channel configuration failed");
                        config_error = true;
                        break;
                    }
                    rank++;
                }

                if (config_error) {
                    continue;
                }

                auto* dma_buffer = reinterpret_cast<uint32_t*>(const_cast<uint16_t*>(buffer));
                if (HAL_ADC_Start_DMA(hadc, dma_buffer, channel_count) != HAL_OK) {
                    PANIC("ADC DMA start failed");
                    continue;
                }

                periph_ready[pidx] = true;
            }

            for (std::size_t i = 0; i < N; ++i) {
                const auto& cfg = runtime_cfgs[i];
                if (!instance_cfg_valid[i]) {
                    continue;
                }
                const auto pidx = peripheral_index(cfg.peripheral);
                instances[i].handle = periph_ready[pidx] ? handle_for(cfg.peripheral) : nullptr;
                instances[i].channel = cfg.channel;
                instances[i].sample_time = cfg.sample_time;
                instances[i].resolution = cfg.resolution;
                instances[i].output = cfg.output;
                instances[i].dma_slot =
                    periph_ready[pidx] ? get_dma_slot(cfg.peripheral, instance_ranks[i]) : nullptr;
            }
        }
    };
};

#else // HAL_ADC_MODULE_ENABLED

struct ADCDomain {
    static constexpr std::size_t max_instances{0};
    struct Entry {};
    struct Config {};
    template <size_t N> static consteval array<Config, N> build(span<const Entry>) { return {}; }
    template <std::size_t N> struct Init {
        static void init(
            std::span<const Config, N>,
            std::span<GPIODomain::Instance> = std::span<GPIODomain::Instance>{},
            std::span<DMADomain::Instance> = std::span<DMADomain::Instance>{}
        ) {}
    };
};

#endif // HAL_ADC_MODULE_ENABLED

#undef STLIB_ADC_DMA_BUFFER_ATTR

} // namespace ST_LIB
