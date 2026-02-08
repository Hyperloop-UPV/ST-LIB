#pragma once

#include "HALAL/hal_wrapper.h"

#include <algorithm>
#include <array>
#include <span>
#include <utility>

#include "ErrorHandler/ErrorHandler.hpp"
#include "HALAL/Models/GPIO.hpp"
#include "HALAL/Models/Pin.hpp"

using std::array;
using std::size_t;
using std::span;

#ifdef HAL_ADC_MODULE_ENABLED
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern ADC_HandleTypeDef hadc3;
#endif

namespace ST_LIB {
extern void compile_error(const char *msg);

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

  enum class ClockPrescaler : uint32_t {
    DIV1 = ADC_CLOCK_ASYNC_DIV1,
    DIV2 = ADC_CLOCK_ASYNC_DIV2,
    DIV4 = ADC_CLOCK_ASYNC_DIV4,
    DIV6 = ADC_CLOCK_ASYNC_DIV6,
    DIV8 = ADC_CLOCK_ASYNC_DIV8,
    DIV10 = ADC_CLOCK_ASYNC_DIV10,
    DIV12 = ADC_CLOCK_ASYNC_DIV12,
    DIV16 = ADC_CLOCK_ASYNC_DIV16,
    DIV32 = ADC_CLOCK_ASYNC_DIV32,
    DIV64 = ADC_CLOCK_ASYNC_DIV64,
    DIV128 = ADC_CLOCK_ASYNC_DIV128,
    DIV256 = ADC_CLOCK_ASYNC_DIV256,
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
    ClockPrescaler prescaler;
    uint32_t sample_rate_hz;
    float *output;
  };

  struct ADC {
    GPIODomain::GPIO gpio;
    using domain = ADCDomain;

    Entry e;

    consteval ADC(const GPIODomain::Pin &pin, float &output,
                  Resolution resolution = Resolution::BITS_12,
                  SampleTime sample_time = SampleTime::CYCLES_8_5,
                  ClockPrescaler prescaler = ClockPrescaler::DIV1,
                  uint32_t sample_rate_hz = 0,
                  Peripheral peripheral = Peripheral::AUTO,
                  Channel channel = Channel::AUTO)
        : gpio{pin, GPIODomain::OperationMode::ANALOG, GPIODomain::Pull::None,
               GPIODomain::Speed::Low},
          e{.gpio_idx = 0,
            .pin = pin,
            .peripheral = peripheral,
            .channel = channel,
            .resolution = resolution,
            .sample_time = sample_time,
            .prescaler = prescaler,
            .sample_rate_hz = sample_rate_hz,
            .output = &output} {
    }

    consteval ADC(const GPIODomain::Pin &pin, Peripheral peripheral,
                  Channel channel, float &output,
                  Resolution resolution = Resolution::BITS_12,
                  SampleTime sample_time = SampleTime::CYCLES_8_5,
                  ClockPrescaler prescaler = ClockPrescaler::DIV1,
                  uint32_t sample_rate_hz = 0)
        : ADC(pin, output, resolution, sample_time, prescaler, sample_rate_hz,
              peripheral, channel) {}

    template <class Ctx> consteval std::size_t inscribe(Ctx &ctx) const {
      const auto gpio_idx = gpio.inscribe(ctx);
      Entry entry = e;
      entry.gpio_idx = gpio_idx;
      const auto resolved = resolve_mapping(entry);
      entry.peripheral = resolved.first;
      entry.channel = resolved.second;
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
    ClockPrescaler prescaler;
    uint32_t sample_rate_hz;
    float *output;
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

  static constexpr std::array<PinMapping, 20> pin_map{{
      {PF11, Peripheral::ADC_1, Channel::CH2, max_bits_adc12},
      {PF12, Peripheral::ADC_1, Channel::CH6, max_bits_adc12},
      {PC0, Peripheral::ADC_1, Channel::CH10, max_bits_adc12},
      {PA0, Peripheral::ADC_1, Channel::CH16, max_bits_adc12},
      {PA3, Peripheral::ADC_1, Channel::CH15, max_bits_adc12},
      {PA4, Peripheral::ADC_1, Channel::CH18, max_bits_adc12},
      {PA5, Peripheral::ADC_1, Channel::CH19, max_bits_adc12},
      {PA6, Peripheral::ADC_1, Channel::CH3, max_bits_adc12},
      {PB0, Peripheral::ADC_1, Channel::CH9, max_bits_adc12},
      {PB1, Peripheral::ADC_1, Channel::CH5, max_bits_adc12},

      {PF13, Peripheral::ADC_2, Channel::CH2, max_bits_adc12},
      {PF14, Peripheral::ADC_2, Channel::CH6, max_bits_adc12},

      {PF5, Peripheral::ADC_3, Channel::CH4, max_bits_adc3},
      {PF6, Peripheral::ADC_3, Channel::CH8, max_bits_adc3},
      {PF7, Peripheral::ADC_3, Channel::CH3, max_bits_adc3},
      {PF8, Peripheral::ADC_3, Channel::CH7, max_bits_adc3},
      {PF9, Peripheral::ADC_3, Channel::CH2, max_bits_adc3},
      {PF10, Peripheral::ADC_3, Channel::CH6, max_bits_adc3},
      {PC2, Peripheral::ADC_3, Channel::CH0, max_bits_adc3},
      {PC3, Peripheral::ADC_3, Channel::CH1, max_bits_adc3},
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

  static consteval std::pair<Peripheral, Channel>
  resolve_mapping(const Entry &e) {
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

    for (const auto &m : pin_map) {
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

  template <size_t N>
  static consteval array<Config, N> build(span<const Entry> entries) {
    static_assert(N <= max_instances, "ADCDomain: too many instances");
    if (entries.size() != N) {
      compile_error("ADC: build entries size mismatch");
    }

    array<Config, N> cfgs{};
    array<bool, 3> periph_seen{};
    array<Resolution, 3> periph_resolution{};
    array<ClockPrescaler, 3> periph_prescaler{};
    array<uint32_t, 3> periph_rate{};
    array<uint8_t, 3> periph_counts{};

    for (std::size_t i = 0; i < N; ++i) {
      const auto &e = entries[i];
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
        compile_error("ADC: sample_rate_hz is not supported in polling mode");
      }

      const auto pidx = peripheral_index(peripheral);

      if (!periph_seen[pidx]) {
        periph_seen[pidx] = true;
        periph_resolution[pidx] = e.resolution;
        periph_prescaler[pidx] = e.prescaler;
        periph_rate[pidx] = e.sample_rate_hz;
      } else {
        if (periph_resolution[pidx] != e.resolution) {
          compile_error("ADC: resolution mismatch on same peripheral");
        }
        if (periph_prescaler[pidx] != e.prescaler) {
          compile_error("ADC: prescaler mismatch on same peripheral");
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
        const auto &prev = entries[j];
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
          .prescaler = e.prescaler,
          .sample_rate_hz = e.sample_rate_hz,
          .output = e.output,
      };
    }

    return cfgs;
  }

  struct Instance {
    ADC_HandleTypeDef *handle = nullptr;
    Channel channel = Channel::CH0;
    SampleTime sample_time = SampleTime::CYCLES_8_5;
    Resolution resolution = Resolution::BITS_12;
    float *output = nullptr;

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

    uint32_t read_raw(uint32_t timeout_ms = 2) {
      if (handle == nullptr) {
        return 0;
      }

      ADC_ChannelConfTypeDef sConfig{};
      sConfig.Channel = static_cast<uint32_t>(channel);
      sConfig.Rank = ADC_REGULAR_RANK_1;
      sConfig.SamplingTime = static_cast<uint32_t>(sample_time);
      sConfig.SingleDiff = ADC_SINGLE_ENDED;
      sConfig.OffsetNumber = ADC_OFFSET_NONE;
      sConfig.Offset = 0;
#if defined(ADC_VER_V5_V90)
      sConfig.OffsetSignedSaturation = DISABLE;
#endif

      if (HAL_ADC_ConfigChannel(handle, &sConfig) != HAL_OK) {
        return 0;
      }
      if (HAL_ADC_Start(handle) != HAL_OK) {
        return 0;
      }
      if (HAL_ADC_PollForConversion(handle, timeout_ms) != HAL_OK) {
        HAL_ADC_Stop(handle);
        return 0;
      }
      const uint32_t val = HAL_ADC_GetValue(handle);
      HAL_ADC_Stop(handle);
      return val;
    }

    void read(double vref = 3.3, uint32_t timeout_ms = 2) {
      if (output == nullptr) {
        return;
      }
      const uint32_t max_val = max_raw_for_resolution(resolution);
      const uint32_t raw = std::min(read_raw(timeout_ms), max_val);
      const double scaled =
          (static_cast<double>(raw) / static_cast<double>(max_val)) * vref;
      *output = static_cast<float>(scaled);
    }
  };

  template <std::size_t N> struct Init {
    static inline std::array<Instance, N> instances{};

    static ADC_HandleTypeDef *handle_for(Peripheral p) {
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

    static void configure_peripheral(const Config &cfg) {
      ADC_HandleTypeDef *hadc = handle_for(cfg.peripheral);

      if (cfg.peripheral == Peripheral::ADC_1 ||
          cfg.peripheral == Peripheral::ADC_2) {
        __HAL_RCC_ADC12_CLK_ENABLE();
        hadc->Instance = (cfg.peripheral == Peripheral::ADC_1) ? ADC1 : ADC2;
      } else {
        __HAL_RCC_ADC3_CLK_ENABLE();
        hadc->Instance = ADC3;
        HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PC2,
                                      SYSCFG_SWITCH_PC2_OPEN);
        HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PC3,
                                      SYSCFG_SWITCH_PC3_OPEN);
      }

      hadc->Init.ClockPrescaler = static_cast<uint32_t>(cfg.prescaler);
      hadc->Init.Resolution = static_cast<uint32_t>(cfg.resolution);
      hadc->Init.ScanConvMode = ADC_SCAN_DISABLE;
      hadc->Init.EOCSelection = ADC_EOC_SINGLE_CONV;
      hadc->Init.LowPowerAutoWait = DISABLE;
      hadc->Init.ContinuousConvMode = DISABLE;
      hadc->Init.NbrOfConversion = 1;
      hadc->Init.DiscontinuousConvMode = DISABLE;
      hadc->Init.NbrOfDiscConversion = 0;
      hadc->Init.ExternalTrigConv = ADC_SOFTWARE_START;
      hadc->Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
      hadc->Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
#if defined(ADC_VER_V5_V90)
      hadc->Init.SamplingMode = ADC_SAMPLING_MODE_NORMAL;
      hadc->Init.DMAContinuousRequests = DISABLE;
#endif
      hadc->Init.Overrun = ADC_OVR_DATA_PRESERVED;
      hadc->Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
      hadc->Init.OversamplingMode = DISABLE;
    }

    static void init(std::span<const Config, N> cfgs,
                     std::span<GPIODomain::Instance> gpio_instances =
                         std::span<GPIODomain::Instance>{}) {
      bool periph_configured[3] = {false, false, false};
      (void)gpio_instances;

      for (std::size_t i = 0; i < N; ++i) {
        const auto &cfg = cfgs[i];
        if (cfg.peripheral == Peripheral::AUTO ||
            cfg.channel == Channel::AUTO) {
          ErrorHandler("ADC config unresolved (AUTO)");
        }
        const auto pidx = peripheral_index(cfg.peripheral);
        if (!periph_configured[pidx]) {
          configure_peripheral(cfg);
          ADC_HandleTypeDef *hadc = handle_for(cfg.peripheral);
          hadc->Init.NbrOfConversion = 1;
          hadc->Init.ScanConvMode = ADC_SCAN_DISABLE;
          hadc->Init.ContinuousConvMode = DISABLE;
          if (HAL_ADC_Init(hadc) != HAL_OK) {
            ErrorHandler("ADC Init failed");
          }

          periph_configured[pidx] = true;
        }

        instances[i].handle = handle_for(cfg.peripheral);
        instances[i].channel = cfg.channel;
        instances[i].sample_time = cfg.sample_time;
        instances[i].resolution = cfg.resolution;
        instances[i].output = cfg.output;
      }
    }
  };
};

#else // HAL_ADC_MODULE_ENABLED

struct ADCDomain {
  static constexpr std::size_t max_instances{0};
  struct Entry {};
  struct Config {};
  template <size_t N>
  static consteval array<Config, N> build(span<const Entry>) {
    return {};
  }
  template <std::size_t N> struct Init {
    static void
    init(std::span<const Config, N>,
         std::span<GPIODomain::Instance> = std::span<GPIODomain::Instance>{}) {}
  };
};

#endif // HAL_ADC_MODULE_ENABLED

} // namespace ST_LIB
