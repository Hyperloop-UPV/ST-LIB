#pragma once

#include "ErrorHandler/ErrorHandler.hpp"
#include "HALAL/Models/GPIO.hpp"
#include "HALAL/Models/Pin.hpp"
#include "HALAL/Models/DMA/DMA2.hpp"
#include "HALAL/Models/MPU.hpp"

#define STLIB_DFSDM_DMA_BUFFER_ATTR(x) D2_NC_BSS_INLINE(x)

#define Oversampling_MAX 1024
#define Oversampling_MAX_Filter_4 215
#define Oversampling_MAX_Filter_5 73
#define Possible_Pin_Channel 20
#define OFFSET_MAX (1 << 23) - 1
#define OFFSET_MIN -(1 << 23)
#define MAX_BUFFER_SIZE_TOTAL 1024
using ST_LIB::DMA_Domain;
using ST_LIB::GPIODomain;
namespace ST_LIB {
using Callback = void (*)(void);
extern void compile_error(const char* msg);

struct DFSDM_CHANNEL_DOMAIN {
    /* Constant Values of Register*/
    // DatPack = 0 Standar
    // DatMPx = 0 // External input
    // ChinSel = 0  // It make sense for our use case.
    // SPICKSel = ¿?
    enum class SPICKSel : uint8_t {
        CLK_IN = 0,
        NORMAL_CLK_OUT = 1,
        CLK_DIVIDED_2_RISING = 2,
        CLK_DIVIDED_2_FALLING = 3
    };
    enum class SPI_Type : uint8_t { SPI_RISING = 0, SPI_FALLING = 1 };

    enum class Filter_Type : uint8_t {
        FastSinc = 0,
        Sinc1 = 1,
        Sinc2 = 2,
        Sinc3 = 3,
        Sinc4 = 4,
        Sinc5 = 5
    };

    enum Fast_Conversion : uint8_t { Disable = 0, Enable = 1 };

    enum class Dma : uint8_t { Disable = 0, Enable = 1 };

    enum class Sync_Conversion : uint8_t { Independent = 0, Sync_With_Flt0 = 1 };

    enum class Regular_Mode : uint8_t { Single = 0, Continuous = 1 };

    enum class Analog_Watchdog_Mode : uint8_t {
        After_Filter = 0, // AWFSEL = 0
        Channel_Data = 1  // AWFSEL = 1
    };
    // AWFSEL = 0 high precision, slow speed <- Ideally for our case
    // AWFSEL = 1 16 bits precision ultra high speed oversampling ratio 1..32 filter (1..3) 8
    // relojes de clock

    enum class Overrun : uint8_t { Enable = 0, Disable = 1 };
    enum class Clock_Absence : uint8_t { Disable = 0, Enable = 1 };
    enum class Short_Circuit : uint8_t { Disable = 0, Enable = 1 };
    enum class Extreme_Detector : uint8_t { Disable = 0, Enable = 1 };
    enum class Analog_Watchdog : uint8_t { Disable = 0, Enable = 1 };
    enum class Trigger_Timer_Source : uint8_t {
        Unused,
        Tim1,
        Tim3,
        Tim4,
        Tim7,
        Tim8,
        Tim16,
        Tim23,
        Tim24,
        Exti11,
        Exti15,
        Lptim1,
        Lptim2,
        Lptim3
    };
    enum class Type_Conversion : uint8_t { Regular, Injected };
    enum class Injected_Mode : uint8_t { Single, Scan };
    struct Config_Channel {
        int32_t offset{0};
        uint32_t right_shift{0};
        SPICKSel spi_clock_sel{SPICKSel::CLK_DIVIDED_2_RISING};
        SPI_Type spi_type{SPI_Type::SPI_RISING};

        /* -------- Runtime protections -------- */
        Clock_Absence clock_absence{Clock_Absence::Disable};
        Short_Circuit short_circuit{Short_Circuit::Disable};
        Extreme_Detector extreme_detector{Extreme_Detector::Disable};

        uint8_t short_circuit_count{0xFF};

        /* -------- Analog watchdog -------- */
        Analog_Watchdog watchdog{Analog_Watchdog::Disable};

        // If Mode Watchdog == After filter
        Filter_Type filter_watchdog{Filter_Type::Sinc2};
        uint8_t watchdog_oversampling{5};
    };
    struct Config_Filter {
        uint8_t filter{0};
        Trigger_Timer_Source trigger_conv{Trigger_Timer_Source::Unused};
        Filter_Type filter_type{Filter_Type::FastSinc};
        uint16_t oversampling{32};
        uint16_t integrator{1};
        Type_Conversion type_conv{Type_Conversion::Regular};
        Dma dma{Dma::Disable};
        Fast_Conversion fast{Fast_Conversion::Disable};
        Sync_Conversion rsync{Sync_Conversion::Independent};
        Regular_Mode rcont{Regular_Mode::Single};
        Injected_Mode jscan{Injected_Mode::Scan};

        Overrun overrun{Overrun::Disable};

        Analog_Watchdog watchdog{Analog_Watchdog::Disable};
        Analog_Watchdog_Mode watchdog_mode{Analog_Watchdog_Mode::After_Filter};
        int32_t watchdog_low_threshold{0x10000000};
        int32_t watchdog_high_threshold{0x7FFFFFFF};
        // callbacks
        Callback watchdog_callback{nullptr};
        Callback conversion_complete_callback{nullptr};
        Callback overrun_callback{nullptr};
        Callback clock_absence_callback{nullptr};
        Callback short_circuit_callback{nullptr};
    };
    static constexpr std::array<std::pair<GPIODomain::Pin, uint8_t>, Possible_Pin_Channel>
        pin_to_channel = {{{PE4, 3},  {PC0, 4},  {PC1, 0},  {PC3, 1},  {PC5, 2},
                           {PB1, 1},  {PF13, 6}, {PE7, 2},  {PE10, 4}, {PE12, 5},
                           {PB10, 7}, {PB12, 1}, {PB14, 2}, {PD9, 3},  {PC7, 3},
                           {PC11, 5}, {PD1, 6},  {PD6, 1},  {PD7, 4},  {PB6, 5}}};

    static consteval uint8_t get_channel(const GPIODomain::Pin& pin) {
        for (int i = 0; i < Possible_Pin_Channel; i++) {
            if (pin_to_channel[i].first == pin) {
                return pin_to_channel[i].second;
            }
        }
        compile_error("This pin cannot be used as a DFSDM_Channel");
    }

    static consteval bool is_correct_oversampling(Filter_Type f, uint16_t osr) {
        switch (f) {
        case Filter_Type::FastSinc:
        case Filter_Type::Sinc1:
        case Filter_Type::Sinc2:
        case Filter_Type::Sinc3:
            return osr >= 1 && osr <= Oversampling_MAX;

        case Filter_Type::Sinc4:
            return osr >= 1 && osr <= Oversampling_MAX_Filter_4;

        case Filter_Type::Sinc5:
            return osr >= 1 && osr <= Oversampling_MAX_Filter_5;
        }
        return false;
    }
    static consteval GPIODomain::AlternateFunction dfsdm_channel_af(const GPIODomain::Pin& pin) {
        if ((pin.port == GPIODomain::Port::B && pin.pin == GPIO_PIN_1) ||
            (pin.port == GPIODomain::Port::B && pin.pin == GPIO_PIN_10) ||
            (pin.port == GPIODomain::Port::B && pin.pin == GPIO_PIN_12) ||
            (pin.port == GPIODomain::Port::B && pin.pin == GPIO_PIN_14) ||
            (pin.port == GPIODomain::Port::C && pin.pin == GPIO_PIN_0)) {
            return GPIODomain::AlternateFunction::AF6;
        }
        if ((pin.port == GPIODomain::Port::C && pin.pin == GPIO_PIN_7) ||
            (pin.port == GPIODomain::Port::D && pin.pin == GPIO_PIN_6)) {
            return GPIODomain::AlternateFunction::AF4;
        }
        if ((pin.port == GPIODomain::Port::B && pin.pin == GPIO_PIN_6)) {
            return GPIODomain::AlternateFunction::AF11;
        }
        return GPIODomain::AlternateFunction::AF3; // In any other case
    }
    static consteval DMA_Domain::Peripheral get_dma_peripheral(uint8_t filter) {
        switch (filter) {
        case 0:
            return DMA_Domain::Peripheral::dfsdm_filter0;
        case 1:
            return DMA_Domain::Peripheral::dfsdm_filter1;
        case 2:
            return DMA_Domain::Peripheral::dfsdm_filter2;
        case 3:
            return DMA_Domain::Peripheral::dfsdm_filter3;
        }
        compile_error("Cannot be different than 0,1,2,3");
    }
    struct Entry {
        Config_Channel config_channel;
        Config_Filter config_filter;
        uint8_t channel;
        size_t gpio_idx;
        size_t buffer_size;
    };

    static constexpr size_t max_instances{8};
    struct DFSDM_CHANNEL {
        using domain = DFSDM_CHANNEL_DOMAIN;
        const GPIODomain::Pin& pin;
        GPIODomain::GPIO gpio;
        const Config_Channel config_channel;
        const Config_Filter config_filter;
        uint8_t channel;
        size_t buffer_size;
        consteval DFSDM_CHANNEL(
            const GPIODomain::Pin& pin,
            const Config_Channel config_channel,
            const Config_Filter config_filter,
            std::size_t buffer_size
        )
            : pin(pin),
              gpio{
                  pin,
                  GPIODomain::OperationMode::ALT_PP,
                  GPIODomain::Pull::None,
                  GPIODomain::Speed::High,
                  dfsdm_channel_af(pin)
              },
              config_channel(config_channel), config_filter(config_filter),
              buffer_size(buffer_size) {
            channel = get_channel(pin);
            if (config_filter.filter > 3) {
                compile_error("Solo hay 4 filtros [0..3]");
            }
            if (config_channel.offset > OFFSET_MAX || config_channel.offset < OFFSET_MIN) {
                compile_error("Your offset is bigger than the maximum size");
            }
            if (config_channel.right_shift > 0x000000FF) {
                compile_error("Your right_shift is bigger than the maximum size");
            }
            if (config_filter.integrator <= 0) {
                compile_error("DFSDM_FILTER: Integrator out of range");
            }
            if (!is_correct_oversampling(config_filter.filter_type, config_filter.oversampling)) {
                compile_error("DFSDM_FILTER: invalid oversampling for selected filter type");
            }
            if (config_channel.watchdog_oversampling > 32) {
                compile_error("DFSDM_Watchdog oversampling is bigger than the maximum allowed");
            }
            if (static_cast<uint32_t>(config_channel.filter_watchdog) > 3) {
                compile_error(
                    "Why would a sane person need a filter of the watchdog higher than sinc3"
                );
            }
        }

        template <class Ctx> consteval std::size_t inscribe(Ctx& ctx) const {
            const auto gpio_idx = gpio.inscribe(ctx);
            Entry e{
                .config_channel = config_channel,
                .config_filter = config_filter,
                .channel = channel,
                .gpio_idx = gpio_idx,
                .buffer_size = buffer_size
            };
            return ctx.template add<DFSDM_CHANNEL_DOMAIN>(e, this);
        }
    };
    // I hate stm32,has volatile in the DFSDM structs.
    struct FilterConfig {
        uint32_t FLTCR1{};
        uint32_t FLTCR2{};
        uint32_t FLTFCR{};
        uint32_t FLTAWHTR{};
        uint32_t FLTAWLTR{};
        uint32_t FLTJCHGR{};
    };
    struct ChannelConfig {
        uint32_t CHCFGR1{};
        uint32_t CHCFGR2{};
        uint32_t CHAWSCDR{};
    };
    struct Config {
        size_t gpio_idx;
        size_t dma_request;
        FilterConfig init_data_filter;
        ChannelConfig init_data_channel;

        uint32_t latency_cycles;
        Type_Conversion type_conv;
        Dma dma_enable;

        uint8_t filter;
        uint8_t channel;

        size_t buffer_size;
        size_t buffer_pos_ini;

        // callbacks
        Callback watchdog_callback{nullptr};
        Callback conversion_complete_callback{nullptr};
        Callback overrun_callback{nullptr};
        Callback clock_absence_callback{nullptr};
        Callback short_circuit_callback{nullptr};
    };
    static consteval DMADomain::Peripheral dma_filter(uint8_t filter) {
        switch (filter) {
        case 0:
            return DMADomain::Peripheral::dfsdm_filter0;
        case 1:
            return DMADomain::Peripheral::dfsdm_filter1;
        case 2:
            return DMADomain::Peripheral::dfsdm_filter2;
        case 3:
            return DMADomain::Peripheral::dfsdm_filter3;
        default:
            compile_error("There is not other filter");
            break;
        }
        return DMADomain::Peripheral::dfsdm_filter0;
    }

    static consteval uint32_t dma_request(uint8_t filter) {
        return DMADomain::get_Request(dma_filter(filter), 0);
    }
    struct BufferSizes {
        std::size_t filter0 = 0;
        std::size_t filter1 = 0;
        std::size_t filter2 = 0;
        std::size_t filter3 = 0;
    };
    static consteval BufferSizes calculate_buffer_sizes(span<const Config> cfgs) {
        BufferSizes sizes;
        for (const auto& cfg : cfgs) {
            switch (cfg.filter) {
            case 0:
                if (cfg.type_conv == Type_Conversion::Regular) {
                    sizes.filter0 = std::max(cfg.buffer_size, sizes.filter0);
                } else {
                    sizes.filter0 += cfg.buffer_size;
                }
                break;
            case 1:
                if (cfg.type_conv == Type_Conversion::Regular) {
                    sizes.filter1 = std::max(cfg.buffer_size, sizes.filter1);
                } else {
                    sizes.filter1 += cfg.buffer_size;
                }
                break;
            case 2:
                if (cfg.type_conv == Type_Conversion::Regular) {
                    sizes.filter2 = std::max(cfg.buffer_size, sizes.filter2);
                } else {
                    sizes.filter2 += cfg.buffer_size;
                }
                break;
            case 3:
                if (cfg.type_conv == Type_Conversion::Regular) {
                    sizes.filter3 = std::max(cfg.buffer_size, sizes.filter3);
                } else {
                    sizes.filter3 += cfg.buffer_size;
                }
                break;
            }
        }
        return sizes;
    }

    static consteval uint32_t compute_latency(const Entry& e) {
        const uint32_t fosr = e.config_filter.oversampling;
        const uint32_t iosr = e.config_filter.integrator;

        if (e.config_filter.fast == Fast_Conversion::Enable &&
            e.config_filter.rcont == Regular_Mode::Continuous) {
            return fosr * iosr;
        }

        if (e.config_filter.filter_type == Filter_Type::FastSinc) {
            return fosr * (iosr - 1 + 4) + 2;
        }

        const uint32_t ford = static_cast<uint32_t>(e.config_filter.filter_type);
        return fosr * (iosr - 1 + ford) + ford;
    }
    static consteval uint32_t get_trigger(Trigger_Timer_Source trig) {
        switch (trig) {
        case Trigger_Timer_Source::Tim1:
            return 0;
        case Trigger_Timer_Source::Tim3:
            return 4;
        case Trigger_Timer_Source::Tim4:
            return 5;
        case Trigger_Timer_Source::Tim7:
            return 8;
        case Trigger_Timer_Source::Tim8:
            return 2;
        case Trigger_Timer_Source::Tim16:
            return 6;
        case Trigger_Timer_Source::Tim23:
            return 11;
        case Trigger_Timer_Source::Tim24:
            return 12;
        case Trigger_Timer_Source::Exti11:
            return 24;
        case Trigger_Timer_Source::Exti15:
            return 25;
        case Trigger_Timer_Source::Lptim1:
            return 26;
        case Trigger_Timer_Source::Lptim2:
            return 27;
        case Trigger_Timer_Source::Lptim3:
            return 28;
        case Trigger_Timer_Source::Unused:
            break;
        }
        return 0;
    }
    // Filter registers
    static consteval uint32_t make_fltfcr(const Entry& e) {
        return (uint32_t(e.config_filter.filter_type) << DFSDM_FLTFCR_FORD_Pos) |
               (uint32_t(e.config_filter.oversampling - 1) << DFSDM_FLTFCR_FOSR_Pos) |
               (uint32_t(e.config_filter.integrator - 1) << DFSDM_FLTFCR_IOSR_Pos);
    }
    static consteval uint32_t make_fltcr2_global() {
        // Activate the interrupt, to activate the continous detection, then the channel can
        return (uint32_t)(DFSDM_FLTCR2_CKABIE | DFSDM_FLTCR2_SCDIE);
    }
    static consteval uint32_t make_fltcr2(const Entry& e) {
        uint32_t v = 0;
        if (e.config_filter.dma == Dma::Disable ||
            e.config_filter.conversion_complete_callback != nullptr) {
            v |= DFSDM_FLTCR2_REOCIE;
            v |= DFSDM_FLTCR2_JEOCIE;
        }
        if (e.config_filter.watchdog == Analog_Watchdog::Enable) {
            v |= DFSDM_FLTCR2_AWDIE;
            v |= 1 << (DFSDM_FLTCR2_AWDCH_Pos + e.channel);
        }
        if (e.config_channel.extreme_detector == Extreme_Detector::Enable) {
            v |= 1 << (DFSDM_FLTCR2_EXCH_Pos + e.channel);
        }
        if (e.config_filter.overrun == Overrun::Enable) {
            v |= DFSDM_FLTCR2_JOVRIE;
            v |= DFSDM_FLTCR2_ROVRIE;
        }
        return v;
    }
    static consteval uint32_t make_fltcr1(const Entry& e, uint32_t filter) {
        uint32_t v = 0;

        if (e.config_filter.type_conv == Type_Conversion::Regular) {
            v |= (uint32_t(e.config_filter.dma) << DFSDM_FLTCR1_RDMAEN_Pos);
            v |= (uint32_t(e.config_filter.fast) << DFSDM_FLTCR1_FAST_Pos);
            v |= (uint32_t(e.config_filter.rsync) << DFSDM_FLTCR1_RSYNC_Pos);
            v |= (uint32_t(e.config_filter.rcont) << DFSDM_FLTCR1_RCONT_Pos);
        } else if (e.config_filter.type_conv == Type_Conversion::Injected) {
            v |= uint32_t(e.config_filter.jscan)
                 << DFSDM_FLTCR1_JSCAN_Pos; // activate conversion of the entire group
            v |= (uint32_t)(e.config_filter.dma) << DFSDM_FLTCR1_JDMAEN_Pos;
            if (e.config_filter.trigger_conv != Trigger_Timer_Source::Unused) {
                v |= DFSDM_FLTCR1_JEXTEN_0; // with the risings
                if (filter == 0) {
                    v |= get_trigger(e.config_filter.trigger_conv) << DFSDM_FLTCR1_JEXTSEL_Pos;
                }
                if (e.config_filter.rsync == Sync_Conversion::Sync_With_Flt0) {
                    v |= DFSDM_FLTCR1_JSYNC;

                } else {
                    v |= get_trigger(e.config_filter.trigger_conv) << DFSDM_FLTCR1_JEXTSEL_Pos;
                }
            }
        }
        v |= uint32_t(e.config_filter.watchdog_mode) << DFSDM_FLTCR1_AWFSEL_Pos;
        return v;
    }
    static consteval uint32_t make_fltawhtr(const Entry& e) {
        uint32_t v = 0;
        if (e.config_filter.watchdog_mode == Analog_Watchdog_Mode::Channel_Data)
            v |= (e.config_filter.watchdog_high_threshold & 0xFFFF)
                 << (DFSDM_FLTAWHTR_AWHT_Pos + DFSDM_FLTAWHTR_AWHT_Pos);
        else
            v |= (e.config_filter.watchdog_high_threshold & 0xFFFFFF) << DFSDM_FLTAWHTR_AWHT_Pos;
        return v;
    }
    static consteval uint32_t make_fltawltr(const Entry& e) {
        uint32_t v = 0;
        if (e.config_filter.watchdog_mode == Analog_Watchdog_Mode::Channel_Data)
            v |= (e.config_filter.watchdog_low_threshold & 0xFFFF)
                 << (DFSDM_FLTAWHTR_AWHT_Pos + DFSDM_FLTAWHTR_AWHT_Pos);
        else
            v |= (e.config_filter.watchdog_low_threshold & 0xFFFFFF) << DFSDM_FLTAWHTR_AWHT_Pos;
        return v;
    }
    // Channel
    static consteval uint32_t make_chawscdr(const Entry& e) {
        uint32_t v = 0;
        v |= uint32_t(e.config_channel.short_circuit_count) << DFSDM_CHAWSCDR_SCDT_Pos;
        if (e.config_channel.watchdog == Analog_Watchdog::Enable) {
            v |= static_cast<uint32_t>(e.config_channel.filter_watchdog)
                 << DFSDM_CHAWSCDR_AWFORD_Pos;
            v |= static_cast<uint32_t>((e.config_channel.watchdog_oversampling - 1) & 0xF)
                 << DFSDM_CHAWSCDR_AWFOSR_Pos;
        }
        return v;
    }
    static consteval uint32_t make_chcfgr1(const Entry& e) {
        uint32_t v = 0;
        // DATPACK = 0  -> Standard mode
        // DATMPX = 0 -> Comes from an external serial input
        // Chinsel = 0 -> channel input are taken from pin of the same channel y
        v |= uint32_t(e.config_channel.spi_clock_sel) << DFSDM_CHCFGR1_SPICKSEL_Pos;
        v |= uint32_t(e.config_channel.spi_type) << DFSDM_CHCFGR1_SITP_Pos;
        v |= uint32_t(e.config_channel.clock_absence) << DFSDM_CHCFGR1_CKABEN_Pos;
        v |= uint32_t(e.config_channel.short_circuit) << DFSDM_CHCFGR1_SCDEN_Pos;

        return v;
    }
    static consteval uint32_t make_chcfgr2(const Entry& e) {
        uint32_t v = 0;
        v |= (e.config_channel.offset & 0x00FFFFFF) << DFSDM_CHCFGR2_OFFSET_Pos;
        v |= uint8_t(e.config_channel.right_shift & 0x0F) << DFSDM_CHCFGR2_DTRBS_Pos;
        return v;
    }

    template <size_t N>
    static consteval std::array<Config, N> build(std::span<const Entry> entries) {
        if (N == 0)
            return {};
        std::array<Config, N> cfgs{};
        std::array<bool, 8> channels_used{false};
        std::array<int8_t, 4> filters_used{-1, -1, -1, -1};
        std::array<int8_t, 8> channel_order{-1, -1, -1, -1, -1, -1, -1, -1};

        for (size_t i = 0; i < N; ++i) {
            const Entry& e = entries[i];

            if (channels_used[e.channel] == true) {
                compile_error("You have two pins using the same channel");
            }
            channels_used[e.channel] = true;
            Config& cfg = cfgs[i];

            cfg.gpio_idx = e.gpio_idx;
            cfg.dma_request = dma_request(e.config_filter.filter);

            cfg.channel = e.channel;
            channel_order[cfg.channel] = i;

            cfg.buffer_size = e.buffer_size;
            cfg.type_conv = e.config_filter.type_conv;
            cfg.dma_enable = e.config_filter.dma;
            cfg.filter = e.config_filter.filter;

            // add the callbacks
            cfg.overrun_callback = e.config_filter.overrun_callback;
            cfg.clock_absence_callback = e.config_filter.clock_absence_callback;
            cfg.short_circuit_callback = e.config_filter.short_circuit_callback;
            cfg.watchdog_callback = e.config_filter.watchdog_callback;
            cfg.conversion_complete_callback = e.config_filter.conversion_complete_callback;

            cfg.init_data_filter.FLTCR1 |= make_fltcr1(e, cfg.filter);
            cfg.init_data_filter.FLTCR2 |= make_fltcr2(e);
            cfg.init_data_filter.FLTFCR |= make_fltfcr(e);
            cfg.init_data_channel.CHCFGR1 |= make_chcfgr1(e);
            cfg.init_data_channel.CHCFGR2 |= make_chcfgr2(e);
            cfg.init_data_channel.CHAWSCDR |= make_chawscdr(e);
            cfg.init_data_filter.FLTAWHTR |= make_fltawhtr(e);
            cfg.init_data_filter.FLTAWLTR |= make_fltawltr(e);
            if (e.config_filter.type_conv == Type_Conversion::Injected)
                cfg.init_data_filter.FLTJCHGR |= 1 << e.channel;
            if (cfg.filter == 0)
                cfg.init_data_filter.FLTCR2 |= make_fltcr2_global();

            cfg.latency_cycles = compute_latency(e);
            if (filters_used[cfg.filter] != -1) {
                if (cfgs[filters_used[cfg.filter]].init_data_filter.FLTCR1 !=
                        cfg.init_data_filter.FLTCR1 ||
                    (cfgs[filters_used[cfg.filter]].init_data_filter.FLTCR2 & 0xFF) !=
                        (cfg.init_data_filter.FLTCR2 & 0xFF) ||
                    cfgs[filters_used[cfg.filter]].init_data_filter.FLTFCR !=
                        cfg.init_data_filter.FLTFCR ||
                    cfgs[filters_used[cfg.filter]].init_data_filter.FLTAWLTR !=
                        cfg.init_data_filter.FLTAWLTR ||
                    cfgs[filters_used[cfg.filter]].init_data_filter.FLTAWHTR !=
                        cfg.init_data_filter.FLTAWHTR ||
                    cfgs[filters_used[cfg.filter]].dma_enable != cfg.dma_enable) {
                    compile_error("You have two channels that goes to the same filter with "
                                  "different filter configuration");
                }
                // have the same thing in every register of the filter
                // Channel group conversion in injected mode
                cfgs[filters_used[cfg.filter]].init_data_filter.FLTJCHGR |=
                    cfg.init_data_filter.FLTJCHGR;
                cfg.init_data_filter.FLTJCHGR =
                    cfgs[filters_used[cfg.filter]].init_data_filter.FLTJCHGR;
                // Watchdog and Extreme detector channel enabled
                cfgs[filters_used[cfg.filter]].init_data_filter.FLTCR2 |=
                    cfg.init_data_filter.FLTCR2;
                cfg.init_data_filter.FLTCR2 =
                    cfgs[filters_used[cfg.filter]].init_data_filter.FLTCR2;
                // Watchdog and Extreme detector channel enabled
            }
            filters_used[cfg.filter] = i;
        }
        // check that dma buffer size is 1 if there is more than one channel per filter and add the
        // buffer_pos_ini to every channel
        std::array<std::array<uint8_t, 8>, 4> channels_filter = {};
        for (size_t i = 0; i < N; i++) {
            channels_filter[cfgs[i].filter][cfgs[i].channel] = cfgs[i].buffer_size;
            uint8_t active_channels = 0;
            // active channels
            for (size_t j = 0; j < 8; j++) {
                if (channels_filter[cfgs[i].filter][j] != 0) {
                    active_channels++;
                }
            }
            // validate buffers
            if (active_channels > 1 &&
                entries[i].config_filter.type_conv == Type_Conversion::Regular) {
                compile_error("Not allowed more than 1 channel per filter in Regular Mode");
            }
            if (active_channels > 1 && cfgs[i].dma_enable == Dma::Enable) {
                // Look the entry because the data that I want to check is easier to access
                if (entries[i].config_filter.jscan == Injected_Mode::Single) {
                    compile_error("Not allowed more than 1 channel per filter + "
                                  "Injected_Mode::Single_conversion");
                }
                for (size_t j = 0; j < 8; j++) {
                    if (channels_filter[cfgs[i].filter][j] > 1) {
                        compile_error("Only allowed buffer_size = 1 when multiple DMA channels are "
                                      "used in the same filter");
                    }
                }
            }
        }
        // give the buffer_pos to every channel I'll give the pos by channel order starting from the
        // low
        std::array<uint8_t, 4> buffer_pos{0, 0, 0, 0};
        for (std::size_t i = 0; i < 8; i++) {
            if (channel_order[i] == -1)
                continue;
            auto& cfg = cfgs[channel_order[i]];
            // If regular conversion give the whole buffer to the channels due to only one channel
            // per filter
            cfg.buffer_pos_ini = buffer_pos[cfg.filter];
            buffer_pos[cfg.filter] += cfg.buffer_size;
        }
        return cfgs;
    }
    static consteval std::size_t
    dma_entries_for_filter(uint8_t filter, std::span<const DMADomain::Entry> dma_entries) {
        std::size_t count = 0;
        for (const auto& entry : dma_entries) {
            if (entry.instance != dma_filter(filter)) {
                continue;
            }
            if (entry.id != 0U) {
                compile_error("DFSDM: DMA for DFSDM filters must use stream id 0");
            }
            ++count;
        }
        return count;
    }
    static consteval bool uses_filter_dma(uint8_t filter, std::span<const Config> cfgs) {
        for (const auto& cfg : cfgs) {
            if (cfg.filter == filter && cfg.dma_enable == Dma::Enable) {
                return true;
            }
        }
        return false;
    }
    static consteval std::size_t dma_contribution_count(
        std::span<const Config> cfgs,
        std::span<const DMADomain::Entry> dma_entries
    ) {
        std::size_t count = 0;
        for (uint8_t fidx = 0; fidx < 4U; ++fidx) {
            if (!uses_filter_dma(fidx, cfgs)) {
                continue;
            }

            const auto existing = dma_entries_for_filter(fidx, dma_entries);
            if (existing > 1U) {
                compile_error("DFSDM: multiple DMA streams configured for the same DFSDM filter");
            }
            if (existing == 0U) {
                ++count;
            }
        }
        return count;
    }
    template <std::size_t ExtraN>
    static consteval std::array<DMADomain::Entry, ExtraN> build_dma_contributions(
        std::span<const DMADomain::Entry> dma_entries,
        std::span<const Config> cfgs
    ) {
        std::array<DMADomain::Entry, ExtraN> extra{};
        std::size_t cursor = 0;

        for (uint8_t fidx = 0; fidx < 4U; ++fidx) {
            if (!uses_filter_dma(fidx, cfgs)) {
                continue;
            }
            const auto existing = dma_entries_for_filter(fidx, dma_entries);
            if (existing == 0U) {
                extra[cursor++] = {
                    .instance = dma_filter(fidx),
                    .stream = DMADomain::Stream::none,
                    .irqn = static_cast<IRQn_Type>(0),
                    .id = 0,
                };
            }
        }

        if (cursor != ExtraN) {
            compile_error("DFSDM: DMA contribution size mismatch");
        }

        return extra;
    }
    static inline uint8_t channels_enabled{};
    static constexpr DFSDM_Filter_TypeDef* filter_hw[4] =
        {DFSDM1_Filter0, DFSDM1_Filter1, DFSDM1_Filter2, DFSDM1_Filter3};
    static constexpr DFSDM_Channel_TypeDef* channel_hw[8] = {
        DFSDM1_Channel0,
        DFSDM1_Channel1,
        DFSDM1_Channel2,
        DFSDM1_Channel3,
        DFSDM1_Channel4,
        DFSDM1_Channel5,
        DFSDM1_Channel6,
        DFSDM1_Channel7
    };
    static void inline start_reg_conv_filter(uint8_t filter) {
        if (filter > 3)
            PANIC("Only filters from 0..3");
        filter_hw[filter]->FLTCR1 |= DFSDM_FLTCR1_RSWSTART; // regular
    }
    static void inline start_inj_conv_filter(uint8_t filter) {
        if (filter > 3)
            PANIC("Only filters from 0..3");
        filter_hw[filter]->FLTCR1 |= DFSDM_FLTCR1_JSWSTART; // injected
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
        GPIODomain::Instance* gpio_instance;
        DMA_Domain::Instance* dma_instance;
        DFSDM_Filter_TypeDef* filter_regs{};
        DFSDM_Channel_TypeDef* channel_regs{};

        Callback watchdog_cb{};
        Callback short_circuit_cb{};
        Callback clock_absence_cb{};
        Callback overrun_cb{};
        Callback end_conversion_cb{};

        uint32_t latency_cycles;
        uint8_t channel;
        uint8_t filter;
        Type_Conversion type_conv;
        Dma dma_enable;

        volatile int32_t* buffer = nullptr;
        size_t length_buffer{};

    private:
        bool is_enabled_channel() const { return (channel_regs->CHCFGR1 & DFSDM_CHCFGR1_CHEN_Msk); }
        bool is_enabled_filter() const { return (filter_regs->FLTCR1 & DFSDM_FLTCR1_DFEN_Msk); }
        bool is_enabled_DFSDM() const {
            return (DFSDM1_Channel0->CHCFGR1 & DFSDM_CHCFGR1_DFSDMEN_Msk);
        }
        void enable_filter() { filter_regs->FLTCR1 |= DFSDM_FLTCR1_DFEN; }
        void enable_channel() { channel_regs->CHCFGR1 |= DFSDM_CHCFGR1_CHEN; }
        void enable_DFSDM_Peripheral() { DFSDM1_Channel0->CHCFGR1 |= DFSDM_CHCFGR1_DFSDMEN; }
        void disable_filter() { filter_regs->FLTCR1 &= ~(DFSDM_FLTCR1_DFEN_Msk); }
        void disable_channel() { channel_regs->CHCFGR1 &= ~(DFSDM_CHCFGR1_CHEN); }
        void disable_DFSDM_Peripheral() { DFSDM1_Channel0->CHCFGR1 &= ~(DFSDM_CHCFGR1_DFSDMEN); }

    public:
        bool is_enabled() {
            return is_enabled_DFSDM() && is_enabled_channel() && is_enabled_filter();
        }
        void enable() {
            // just in case enable everything to work
            enable_DFSDM_Peripheral();
            enable_channel();
            enable_filter();
        }

        void disable() {
            // only disable channel
            channel_regs->CHCFGR1 &= ~(DFSDM_CHCFGR1_CHEN_Msk);
        }

        void enable_clock_absence_detector() { channel_regs->CHCFGR1 |= DFSDM_CHCFGR1_CKABEN; }
        void enable_short_circuit_detector() { channel_regs->CHCFGR1 |= DFSDM_CHCFGR1_SCDEN; }
        void disable_clock_absence_detector() { channel_regs->CHCFGR1 &= (~DFSDM_CHCFGR1_CKABEN); }
        void disable_short_circuit_detector() { channel_regs->CHCFGR1 &= (~DFSDM_CHCFGR1_SCDEN); }
        void enable_overrun() { filter_regs->FLTCR2 |= DFSDM_FLTCR2_ROVRIE; }
        void disable_overrun() { filter_regs->FLTCR2 &= (~DFSDM_FLTCR2_ROVRIE); }
        void enable_watchdog() { filter_regs->FLTCR2 |= DFSDM_FLTCR2_AWDCH; }
        void disable_watchdog() { filter_regs->FLTCR2 &= (~DFSDM_FLTCR2_AWDCH); }
        /*channel functions */
        void change_offset(int32_t offset) {
            channel_regs->CHCFGR2 &= ~(DFSDM_CHCFGR2_OFFSET_Msk);
            channel_regs->CHCFGR2 |= (offset & 0x00FFFFFF) << DFSDM_CHCFGR2_OFFSET_Pos;
        }

        /*Filter functions*/

        void modify_sync_conversion(Sync_Conversion type) {
            bool was_enabled_filter = is_enabled_filter();
            if (was_enabled_filter)
                disable_filter();

            filter_regs->FLTCR1 &= ~DFSDM_FLTCR1_RSYNC_Msk;
            filter_regs->FLTCR1 |= (uint32_t(type) << DFSDM_FLTCR1_RSYNC_Pos);

            if (was_enabled_filter)
                enable_filter();
        }
        void modify_regular_mode(Regular_Mode mode) {
            bool was_enabled_filter = is_enabled_filter();
            if (was_enabled_filter)
                disable_filter();

            filter_regs->FLTCR1 &= ~DFSDM_FLTCR1_RCONT_Msk;
            filter_regs->FLTCR1 |= (uint32_t(mode) << DFSDM_FLTCR1_RCONT_Pos);

            if (was_enabled_filter)
                enable_filter();
        }

        bool modify_oversampling(uint16_t oversampling) {
            if (oversampling == 0)
                return false;

            uint32_t ford = (filter_regs->FLTFCR & DFSDM_FLTFCR_FORD_Msk) >> DFSDM_FLTFCR_FORD_Pos;

            if (ford <= 3 && oversampling > Oversampling_MAX)
                return false;
            if (ford == 4 && oversampling > Oversampling_MAX_Filter_4)
                return false;
            if (ford == 5 && oversampling > Oversampling_MAX_Filter_5)
                return false;

            bool was_enabled_filter = is_enabled_filter();
            if (was_enabled_filter)
                disable_filter();

            filter_regs->FLTFCR &= ~DFSDM_FLTFCR_FOSR_Msk;
            filter_regs->FLTFCR |= ((uint32_t)(oversampling - 1) << DFSDM_FLTFCR_FOSR_Pos);
            if (was_enabled_filter)
                enable_filter();
            return true;
        }

        bool modify_integrator(uint8_t integrator) {

            if (integrator == 0 || integrator > 256)
                return false;
            bool was_enabled_filter = is_enabled_filter();
            if (was_enabled_filter)
                disable_filter();

            filter_regs->FLTFCR &= ~DFSDM_FLTFCR_IOSR_Msk;
            filter_regs->FLTFCR |= ((integrator - 1) << DFSDM_FLTFCR_IOSR_Pos);

            if (was_enabled_filter)
                enable_filter();
            return true;
        }

        bool modify_filter_order(Filter_Type type) {

            uint32_t fosr =
                ((filter_regs->FLTFCR & DFSDM_FLTFCR_FOSR_Msk) >> DFSDM_FLTFCR_FOSR_Pos);

            if (type == Filter_Type::Sinc4 && fosr > Oversampling_MAX_Filter_4)
                return false;
            if (type == Filter_Type::Sinc5 && fosr > Oversampling_MAX_Filter_5)
                return false;

            bool was_enabled_filter = is_enabled_filter();
            if (was_enabled_filter)
                disable_filter();

            filter_regs->FLTFCR &= ~DFSDM_FLTFCR_FORD_Msk;
            filter_regs->FLTFCR |= (uint32_t(type) << DFSDM_FLTFCR_FORD_Pos);

            if (was_enabled_filter)
                enable_filter();
            return true;
        }
        int32_t read(size_t pos) {
            if (pos >= this->length_buffer) {
                PANIC("DFSDM: Trying to access to a memory section that is not from the channel "
                      "buffer");
            }
            return (
                static_cast<int32_t>(this->buffer[pos] & DFSDM_FLTJDATAR_JDATA_Msk) >>
                DFSDM_FLTJDATAR_JDATA_Pos
            ); // The constants values are the same for regular than injected
        }
        int32_t read() {
            return (
                static_cast<int32_t>(this->buffer[0] & DFSDM_FLTJDATAR_JDATA_Msk) >>
                DFSDM_FLTJDATAR_JDATA_Pos
            );
        }
        uint32_t check_latency_cycles() {
            return filter_regs->FLTCNVTIMR >> DFSDM_FLTCNVTIMR_CNVCNT_Pos;
        }

        uint32_t check_min_extreme_detector() {
            return filter_regs->FLTEXMIN >> DFSDM_FLTEXMIN_EXMIN_Pos;
        }

        uint32_t check_max_extreme_detector() {
            return filter_regs->FLTEXMAX >> DFSDM_FLTEXMAX_EXMAX_Pos;
        }
        void modify_watchdog_lth(uint32_t value) {

            filter_regs->FLTAWLTR &= ~DFSDM_FLTAWLTR_AWLT_Msk;
            bool fast = (filter_regs->FLTCR1 & DFSDM_FLTCR1_AWFSEL);

            if (fast)
                filter_regs->FLTAWLTR =
                    (value & 0xFFFF)
                    << (DFSDM_FLTAWLTR_AWLT_Pos + DFSDM_FLTAWLTR_AWLT_Pos); // Only 16 bits
            else
                filter_regs->FLTAWLTR = (value & 0xFFFFFF) << DFSDM_FLTAWLTR_AWLT_Pos; // 24 bits
        }

        void modify_watchdog_hth(uint32_t value) {
            filter_regs->FLTAWHTR &= ~DFSDM_FLTAWHTR_AWHT_Msk;
            bool fast = (filter_regs->FLTCR1 & DFSDM_FLTCR1_AWFSEL);

            if (fast)
                filter_regs->FLTAWHTR = (value & 0xFFFF)
                                        << (DFSDM_FLTAWHTR_AWHT_Pos + DFSDM_FLTAWHTR_AWHT_Pos);
            else
                filter_regs->FLTAWHTR = (value & 0xFFFFFF) << DFSDM_FLTAWHTR_AWHT_Pos;
        }
        // get the last conversion from a filter
        static uint8_t get_last_conversion_from_filter(uint8_t filter, Type_Conversion conv) {
            uint8_t channel = 0xFF;
            switch (filter) {
            case 0:
                if (conv == Type_Conversion::Injected) {
                    channel = (DFSDM1_Filter0->FLTJDATAR & 0x7);
                } else {
                    channel = (DFSDM1_Filter0->FLTRDATAR & 0x7);
                }
                break;
            case 1:
                if (conv == Type_Conversion::Injected) {
                    channel = (DFSDM1_Filter1->FLTJDATAR & 0x7);
                } else {
                    channel = (DFSDM1_Filter1->FLTRDATAR & 0x7);
                }
                break;

            case 2:
                if (conv == Type_Conversion::Injected) {
                    channel = (DFSDM1_Filter2->FLTJDATAR & 0x7);
                } else {
                    channel = (DFSDM1_Filter2->FLTRDATAR & 0x7);
                }
                break;
            case 3:
                if (conv == Type_Conversion::Injected) {
                    channel = (DFSDM1_Filter3->FLTJDATAR & 0x7);
                } else {
                    channel = (DFSDM1_Filter3->FLTRDATAR & 0x7);
                }
                break;
            default:
                break;
            }
            return channel;
        }
    };
    static inline Instance* channel_instances[DFSDM_CHANNEL_DOMAIN::max_instances] = {nullptr};
    template <std::size_t N, std::array<Config, N> cfgs> struct Init {
        static constexpr auto sizes = calculate_buffer_sizes(cfgs);
        // calculamos tamaño tanto para filtros con DMA como para los que no tienen
        static constexpr std::size_t total_slots =
            sizes.filter0 + sizes.filter1 + sizes.filter2 + sizes.filter3;

        // Filter Buffers
        alignas(32) STLIB_DFSDM_DMA_BUFFER_ATTR(Buffer_Filter0
        ) static int32_t Buffer_Filter0[sizes.filter0 > 0 ? sizes.filter0 : 1]{};
        alignas(32) STLIB_DFSDM_DMA_BUFFER_ATTR(Buffer_Filter1
        ) static int32_t Buffer_Filter1[sizes.filter1 > 0 ? sizes.filter1 : 1]{};
        alignas(32) STLIB_DFSDM_DMA_BUFFER_ATTR(Buffer_Filter2
        ) static int32_t Buffer_Filter2[sizes.filter2 > 0 ? sizes.filter2 : 1]{};
        alignas(32) STLIB_DFSDM_DMA_BUFFER_ATTR(Buffer_Filter3
        ) static int32_t Buffer_Filter3[sizes.filter3 > 0 ? sizes.filter3 : 1]{};

        static inline std::array<Instance, N> instances{};

        static constexpr std::size_t buffer_size_for(uint8_t filter) {
            switch (filter) {
            case 0:
                return sizes.filter0;
            case 1:
                return sizes.filter1;
            case 2:
                return sizes.filter2;
            case 3:
                return sizes.filter3;
            }
            PANIC("Filter cannot be bigger than 3");
            return 0;
        }
        static volatile int32_t* get_buffer_filter(uint8_t filter) {
            switch (filter) {
            case 0:
                return Buffer_Filter0;
            case 1:
                return Buffer_Filter1;
            case 2:
                return Buffer_Filter2;
            case 3:
                return Buffer_Filter3;
            }
            PANIC("Filter cannot be bigger than 3");
            return 0;
        }

        static volatile int32_t* get_buffer_pointer(const Config cfg) {
            return get_buffer_filter(cfg.filter) + cfg.buffer_pos_ini;
        }

        static void init(
            std::span<GPIODomain::Instance> gpio_instances,
            std::span<DMA_Domain::Instance> dma_instances
        ) {
            if (N == 0)
                return;
            std::array<bool, 4> filters_configured = {false, false, false, false};
            RCC->APB2ENR |= RCC_APB2ENR_DFSDM1EN; // Activate the DFSDM clock
            for (size_t i = 0; i < N; ++i) {
                const Config& cfg = cfgs[i];
                filter_hw[cfg.filter]->FLTCR1 &= ~DFSDM_FLTCR1_DFEN;
                channel_hw[cfg.channel]->CHCFGR1 &= ~DFSDM_CHCFGR1_CHEN;
            }
            for (std::size_t i = 0; i < N; ++i) {
                const Config& cfg = cfgs[i];
                Instance& inst = instances[i];
                inst.gpio_instance = &gpio_instances[cfg.gpio_idx];
                if (cfg.dma_enable == Dma::Enable) {
                    inst.dma_instance = find_dma_instance(cfg.dma_request, dma_instances);
                } else {
                    inst.dma_instance = nullptr;
                }

                inst.filter_regs = filter_hw[cfg.filter];
                inst.channel_regs = channel_hw[cfg.channel];

                inst.latency_cycles = cfg.latency_cycles;
                inst.type_conv = cfg.type_conv;
                inst.filter = cfg.filter;
                inst.channel = cfg.channel;
                inst.dma_enable = cfg.dma_enable;

                inst.length_buffer = cfg.buffer_size;
                inst.buffer = get_buffer_pointer(cfg);
                // callbacks
                inst.overrun_cb = cfg.overrun_callback;
                inst.short_circuit_cb = cfg.short_circuit_callback;
                inst.watchdog_cb = cfg.watchdog_callback;
                inst.end_conversion_cb = cfg.conversion_complete_callback;

                if (!filters_configured[cfg.filter]) {
                    // add everything to the register of the filter
                    inst.filter_regs->FLTCR1 |= cfg.init_data_filter.FLTCR1;
                    if (inst.type_conv == Type_Conversion::Regular) {
                        inst.filter_regs->FLTCR1 &= ~DFSDM_FLTCR1_RCH_Msk;
                        inst.filter_regs->FLTCR1 |= uint32_t(inst.channel) << DFSDM_FLTCR1_RCH_Pos;
                    }
                    inst.filter_regs->FLTCR2 |= cfg.init_data_filter.FLTCR2;
                    inst.filter_regs->FLTFCR |= cfg.init_data_filter.FLTFCR;
                    inst.filter_regs->FLTAWHTR |= cfg.init_data_filter.FLTAWHTR;
                    inst.filter_regs->FLTAWLTR |= cfg.init_data_filter.FLTAWLTR;
                    inst.filter_regs->FLTJCHGR = cfg.init_data_filter.FLTJCHGR;

                    filters_configured[cfg.filter] = true;

                    // add dma
                    if (cfg.dma_enable == Dma::Enable) {
                        const std::uintptr_t src_address =
                            inst.type_conv == Type_Conversion::Regular
                                ? reinterpret_cast<std::uintptr_t>(
                                      &filter_hw[inst.filter]->FLTRDATAR
                                  )
                                : reinterpret_cast<std::uintptr_t>(
                                      &filter_hw[inst.filter]->FLTJDATAR
                                  );
                        inst.dma_instance->start(
                            static_cast<uint32_t>(src_address),
                            static_cast<uint32_t>(
                                reinterpret_cast<std::uintptr_t>(get_buffer_filter(inst.filter))
                            ),
                            buffer_size_for(inst.filter)
                        );
                    }
                }
                // add everything to the channel register
                inst.channel_regs->CHCFGR1 |= cfg.init_data_channel.CHCFGR1;
                inst.channel_regs->CHCFGR2 |= cfg.init_data_channel.CHCFGR2;
                inst.channel_regs->CHAWSCDR |= cfg.init_data_channel.CHAWSCDR;

                // update channel_instances
                channel_instances[inst.channel] = &inst;
                channels_enabled |= 1 << inst.channel;
            }
            if (N > 0) {
                ;
                // Activate the DFSDM GLOBAL Interface
                DFSDM1_Channel0->CHCFGR1 |= DFSDM_CHCFGR1_DFSDMEN;
                for (int i = 0; i < 8; i++) {
                    if (channels_enabled & (1 << i)) {
                        channel_hw[i]->CHCFGR1 |= DFSDM_CHCFGR1_CHEN;
                    }
                }
                for (int i = 0; i < 4; i++) {
                    if (filters_configured[i] == true) {
                        filter_hw[i]->FLTCR1 |= DFSDM_FLTCR1_DFEN;
                    }
                }
                // activate the NVIC
                for (int i = 0; i < 4; i++) {
                    if (filters_configured[i] == true) {
                        switch (i) {
                        case 0:
                            NVIC_EnableIRQ(DFSDM1_FLT0_IRQn);
                            break;
                        case 1:
                            NVIC_EnableIRQ(DFSDM1_FLT1_IRQn);
                            break;
                        case 2:
                            NVIC_EnableIRQ(DFSDM1_FLT2_IRQn);
                            break;
                        case 3:
                            NVIC_EnableIRQ(DFSDM1_FLT3_IRQn);
                            break;
                        }
                    }
                }
            }
        }
    };
    static inline volatile size_t idx_filter[4] = {};
    static void handle_irq(uint8_t filter_index) {

        DFSDM_Filter_TypeDef* filter = filter_hw[filter_index];

        uint32_t isr = filter->FLTISR;

        if (isr & DFSDM_FLTISR_REOCF_Msk) {
            // Save it in the address provide by the user
            int32_t data = filter->FLTRDATAR;
            Instance* inst = channel_instances
                [(data & DFSDM_FLTRDATAR_RDATACH_Msk) >> DFSDM_FLTRDATAR_RDATACH_Pos];
            if (inst != nullptr && inst->buffer != nullptr) {
                if (inst->dma_enable == Dma::Disable) [[likely]] {
                    inst->buffer[idx_filter[inst->filter]] = data;
                    idx_filter[inst->filter] = (idx_filter[inst->filter] + 1) % inst->length_buffer;
                }
                if (inst->end_conversion_cb != nullptr) {
                    inst->end_conversion_cb();
                }
            }
        }
        if (isr & DFSDM_FLTISR_JEOCF_Msk) {
            // Save it in the address provide by the user
            int32_t data = filter->FLTJDATAR;
            uint8_t channel = (data & DFSDM_FLTJDATAR_JDATACH_Msk);
            Instance* inst = channel_instances[channel];
            if (inst != nullptr && inst->buffer != nullptr) {
                if (inst->dma_enable == Dma::Disable) [[likely]] {
                    inst->buffer[idx_filter[inst->filter]] = data;
                    idx_filter[inst->filter] = (idx_filter[inst->filter] + 1) % inst->length_buffer;
                }
                if (inst->end_conversion_cb != nullptr) {
                    inst->end_conversion_cb();
                }
            }
        }
        if (isr & DFSDM_FLTISR_ROVRF_Msk) {
            Instance* inst = channel_instances[filter->FLTRDATAR & DFSDM_FLTRDATAR_RDATACH_Msk];
            if (inst != nullptr && inst->overrun_cb != nullptr)
                inst->overrun_cb();
            // clear
            filter->FLTICR |= DFSDM_FLTICR_CLRROVRF_Msk;
        }
        if (isr & DFSDM_FLTISR_JOVRF_Msk) {
            Instance* inst = channel_instances[filter->FLTJDATAR & DFSDM_FLTJDATAR_JDATACH_Msk];
            if (inst != nullptr && inst->overrun_cb != nullptr)
                inst->overrun_cb();
            // clear
            filter->FLTICR |= DFSDM_FLTICR_CLRJOVRF_Msk;
        }
        if (isr & (channels_enabled << DFSDM_FLTICR_CLRSCDF_Pos)) {
            uint32_t ch = __builtin_ctz((isr & DFSDM_FLTISR_SCDF_Msk) >> DFSDM_FLTISR_SCDF_Pos);
            if (channel_instances[ch] != nullptr &&
                channel_instances[ch]->short_circuit_cb != nullptr)
                channel_instances[ch]->short_circuit_cb();
            // clear
            filter->FLTICR |= DFSDM_FLTICR_CLRSCDF;
        }
        if (isr & (channels_enabled << DFSDM_FLTISR_CKABF_Pos)) {
            uint32_t ch = __builtin_ctz((isr & DFSDM_FLTISR_CKABF_Msk) >> DFSDM_FLTISR_CKABF_Pos);
            if (channel_instances[ch] != nullptr &&
                channel_instances[ch]->clock_absence_cb != nullptr)
                channel_instances[ch]->clock_absence_cb();
            // clear
            filter->FLTICR |= DFSDM_FLTICR_CLRCKABF;
        }
        // Analog watchdog
        if (isr & (DFSDM_FLTISR_AWDF << DFSDM_FLTISR_AWDF_Pos)) {
            if (filter->FLTAWSR & DFSDM_FLTAWSR_AWHTF_Msk) {
                uint32_t ch = __builtin_ctz(filter->FLTAWSR & DFSDM_FLTAWSR_AWHTF_Msk);
                if (channel_instances[ch] != nullptr &&
                    channel_instances[ch]->watchdog_cb != nullptr)
                    channel_instances[ch]->watchdog_cb();
                // clear
                filter->FLTAWCFR = DFSDM_FLTAWCFR_CLRAWHTF;
            }
            if (filter->FLTAWSR & DFSDM_FLTAWSR_AWLTF_Msk) {
                uint32_t ch = __builtin_ctz(filter->FLTAWSR & DFSDM_FLTAWSR_AWLTF_Msk);
                if (channel_instances[ch] != nullptr &&
                    channel_instances[ch]->watchdog_cb != nullptr)
                    channel_instances[ch]->watchdog_cb();
                // clear
                filter->FLTAWCFR = DFSDM_FLTAWCFR_CLRAWLTF;
            }
        }
    }
};
struct DFSDM_CLK_DOMAIN {
    static constexpr GPIODomain::Pin valid_clk_pins[] = {
        {GPIODomain::Port::C, GPIO_PIN_2},
        {GPIODomain::Port::B, GPIO_PIN_0},
        {GPIODomain::Port::E, GPIO_PIN_9},
        {GPIODomain::Port::D, GPIO_PIN_3},
        {GPIODomain::Port::D, GPIO_PIN_10}
    };

    static consteval bool is_valid_dfsdm_clk_pin(GPIODomain::Port port, uint32_t pin) {
        bool found = false;
        for (auto& p : valid_clk_pins) {
            if (p.port == port && p.pin == pin) {
                found = true;
                break;
            }
        }
        return found;
    }

    static consteval GPIODomain::AlternateFunction dfsdm_clk_af(const GPIODomain::Pin& pin) {
        if ((pin.port == GPIODomain::Port::C && pin.pin == GPIO_PIN_2) ||
            (pin.port == GPIODomain::Port::B && pin.pin == GPIO_PIN_0))
            return GPIODomain::AlternateFunction::AF6;
        return GPIODomain::AlternateFunction::AF3; // In every other case
    }
    struct Entry {
        size_t gpio_idx;
        uint16_t clk_divider;
    };

    struct DFSDM_CLK {
        using domain = DFSDM_CLK_DOMAIN;
        GPIODomain::GPIO gpio;
        GPIODomain::Pin pin;
        uint8_t clk_divider;
        consteval DFSDM_CLK(const GPIODomain::Pin& pin, uint8_t clk_divider = 100)
            : // clk_divider = 100 -> 1Mhz
              gpio{
                  pin,
                  GPIODomain::OperationMode::ALT_PP,
                  GPIODomain::Pull::None,
                  GPIODomain::Speed::High,
                  dfsdm_clk_af(pin)
              },
              pin(pin), clk_divider(clk_divider) {}

        template <class Ctx> consteval std::size_t inscribe(Ctx& ctx) const {
            const auto gpio_idx = gpio.inscribe(ctx);
            if (!is_valid_dfsdm_clk_pin(pin.port, pin.pin)) {
                compile_error("Invalid clk dfsdm pin used");
            }
            if (clk_divider < 7 || clk_divider > 256) {
                compile_error("The clk_divider has to be between 7 and 256");
            }
            Entry e{.gpio_idx = gpio_idx, .clk_divider = clk_divider};
            return ctx.template add<DFSDM_CLK_DOMAIN>(e, this);
        }
    };
    static constexpr std::size_t max_instances{1};
    struct Config {
        size_t gpio_idx;
        uint16_t clk_divider;
    };
    template <size_t N>
    static consteval std::array<Config, N> build(std::span<const Entry> entries) {
        std::array<Config, N> cfgs{};
        static_assert(N <= 1, "You can't have more than one clock_out");
        for (std::size_t i = 0; i < N; ++i) {
            cfgs[i] = {.gpio_idx = entries[i].gpio_idx, .clk_divider = entries[i].clk_divider};
        }
        return cfgs;
    }
    struct Instance {
        GPIODomain::Instance* gpio_instance;
        uint16_t clk_divider;
        /*Already called in init()*/
        void init() {
            RCC->APB2ENR |= RCC_APB2ENR_DFSDM1EN; // Activate the DFSDM Clock OUT in RCC by default
                                                  // it uses rcc_pclk2
            // Disable DFSDMEN to change parameters
            DFSDM1_Channel0->CHCFGR1 &= ~DFSDM_CHCFGR1_DFSDMEN;

            // CKOUTSRC = 0 -> kernel clock (rcc_pclk2)  It works 137,5 Mhz,
            DFSDM1_Channel0->CHCFGR1 &= ~DFSDM_CHCFGR1_CKOUTSRC;
            // CKOUT Divider. Divider = CKOUTDIV + 1
            DFSDM1_Channel0->CHCFGR1 &= ~DFSDM_CHCFGR1_CKOUTDIV_Msk;

            DFSDM1_Channel0->CHCFGR1 |= uint32_t(clk_divider - 1) << DFSDM_CHCFGR1_CKOUTDIV_Pos;

            // enable the DFSDM Global Interface
            DFSDM1_Channel0->CHCFGR1 |= DFSDM_CHCFGR1_DFSDMEN;
        }
        bool disable() {
            DFSDM1_Channel0->CHCFGR1 &= ~DFSDM_CHCFGR1_DFSDMEN;
            return (DFSDM1_Channel0->CHCFGR1 & DFSDM_CHCFGR1_DFSDMEN) == 0;
        }
        bool enable() {
            DFSDM1_Channel0->CHCFGR1 |= DFSDM_CHCFGR1_DFSDMEN;
            return (DFSDM1_Channel0->CHCFGR1 & DFSDM_CHCFGR1_DFSDMEN);
        }
        bool change_divider(uint8_t div) {
            if (div < 4)
                return false;
            clk_divider = div;
            if (disable()) {
                init();
                return true;
            }
            return false;
        }
    };
    template <std::size_t N> struct Init {
        static inline std::array<Instance, N> instances{};
        static void
        init(std::span<const Config, N> cfgs, std::span<GPIODomain::Instance> gpio_instances) {
            if (N == 0)
                return;
            const auto& c = cfgs[0];
            auto& inst = instances[0];
            inst.gpio_instance = &gpio_instances[c.gpio_idx];
            inst.clk_divider = c.clk_divider;
            inst.init();
        }
    };
};

}; // namespace ST_LIB
