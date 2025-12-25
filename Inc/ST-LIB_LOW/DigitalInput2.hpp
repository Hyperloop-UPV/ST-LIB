#pragma once

#include "HALAL/Models/GPIO.hpp"

using ST_LIB::GPIODomain;

namespace ST_LIB {
struct DigitalInputDomain {
  struct Entry {
    size_t gpio_idx;
  };
  struct DigitalInput {
    GPIODomain::GPIO gpio;
    using domain = DigitalInputDomain;

    consteval DigitalInput(const GPIODomain::Pin &pin,
                           GPIODomain::Pull pull = GPIODomain::Pull::None,
                           GPIODomain::Speed speed = GPIODomain::Speed::Low)
        : gpio{pin, GPIODomain::OperationMode::INPUT, pull, speed} {}

    template <class Ctx> consteval auto inscribe(Ctx &ctx) const {
      const auto gpio_idx = gpio.inscribe(ctx);
      Entry e{.gpio_idx = gpio_idx};
      return ctx.template add<DigitalInputDomain>(e, this);
    }
  };

  static constexpr std::size_t max_instances{110};

  struct Config {
    size_t gpio_idx;
  };

  template <size_t N>
  static consteval array<Config, N> build(span<const Entry> outputs) {
    array<Config, N> cfgs{};

    for (std::size_t i = 0; i < N; ++i) {
      cfgs[i].gpio_idx = outputs[i].gpio_idx;
    }
    return cfgs;
  }

  struct Instance {
    GPIODomain::Instance *gpio_instance;

    GPIO_PinState read() { return gpio_instance->read(); }
  };

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
