#pragma once

#include "HALAL/Models/GPIO.hpp"
#include "stm32h7xx_ll_adc.h"
#include <array>
#include <span>

using std::array;
using std::size_t;
using std::span;

namespace ST_LIB {
extern void compile_error(const char *msg);

struct ADCDomain {
  enum class Resolution : uint32_t {
    BITS_16 = LL_ADC_RESOLUTION_16B,
    BITS_14 = LL_ADC_RESOLUTION_14B,
    BITS_12 = LL_ADC_RESOLUTION_12B,
    BITS_10 = LL_ADC_RESOLUTION_10B,
    BITS_8 = LL_ADC_RESOLUTION_8B,
    BITS_6 = LL_ADC_RESOLUTION_6B,
  };
  enum class Mode : uint32_t {
    SINGLE = LL_ADC_REG_CONV_SINGLE,
    CONTINUOUS = LL_ADC_REG_CONV_CONTINUOUS,
  };
  enum class Delivery : uint32_t { DMA, POLLING };
  struct Entry {
    size_t gpio_idx;

    Resolution res;
    uint32_t sampleRate;
    Mode mode;
    Delivery delivery;
  };

  struct ADC {
    GPIODomain::GPIO gpio;
    using domain = ADCDomain;

    Entry e;

    consteval ADC(const GPIODomain::Pin &pin,
                  Resolution res = Resolution::BITS_16, uint32_t sampleRate = 0,
                  Mode mode = Mode::SINGLE, Delivery delivery = Delivery::DMA)
        : gpio(pin, GPIODomain::OperationMode::ANALOG, GPIODomain::Pull::None,
               GPIODomain::Speed::Low),
          e{.res = res,
            .sampleRate = sampleRate,
            .mode = mode,
            .delivery = delivery} {}

    template <class Ctx> consteval void inscribe(Ctx &ctx) const {
      const auto gpio_idx = ctx.template add<GPIODomain>(gpio.e);
      Entry entry = e;
      entry.gpio_idx = gpio_idx;
      ctx.template add<ADCDomain>(entry);
    }
  };

  static constexpr std::size_t max_instances{16};

  struct Config {};

  template <size_t N>
  static consteval array<Config, N> build(span<const Entry> adcs) {
    array<Config, N> cfgs{};

    for (std::size_t i = 0; i < N; ++i) {
      cfgs[i].gpio_idx = adcs[i].gpio_idx;
    }
    return cfgs;
  }

  // Runtime object
  struct Instance {};

  template <std::size_t N> struct Init {
    static inline std::array<Instance, N> instances{};

    static void init(std::span<const Config, N> cfgs,
                     std::span<GPIODomain::Instance> gpio_instances) {
      for (std::size_t i = 0; i < N; ++i) {
        const auto &e = cfgs[i];

        instances[i].gpio_instance = &gpio_instances[e.gpio_idx];
      }
    }
  };
};
} // namespace ST_LIB