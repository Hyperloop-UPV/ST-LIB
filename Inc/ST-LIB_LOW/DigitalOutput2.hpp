#pragma once

#include "HALAL/Models/GPIO.hpp"

using ST_LIB::GPIODomain;

namespace ST_LIB {
struct DigitalOutputDomain {
  enum class OutputMode : uint8_t {
    PUSH_PULL =
        static_cast<uint8_t>(GPIODomain::OperationMode::OUTPUT_PUSHPULL),
    OPEN_DRAIN =
        static_cast<uint8_t>(GPIODomain::OperationMode::OUTPUT_OPENDRAIN),
  };
  struct Entry {
    size_t gpio_idx;
  };
  struct DigitalOutput {
    GPIODomain::GPIO gpio;
    using domain = DigitalOutputDomain;

    consteval DigitalOutput(const GPIODomain::Pin &pin,
                            OutputMode mode = OutputMode::PUSH_PULL,
                            GPIODomain::Pull pull = GPIODomain::Pull::None,
                            GPIODomain::Speed speed = GPIODomain::Speed::Low)
        : gpio{pin, static_cast<GPIODomain::OperationMode>(mode), pull, speed} {
    }

    template <class Ctx> consteval auto inscribe(Ctx &ctx) const {
      const auto gpio_idx = gpio.inscribe(ctx);
      Entry e{.gpio_idx = gpio_idx};
      return ctx.template add<DigitalOutputDomain>(e, this);
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

    void turn_on() { gpio_instance->turn_on(); }

    void turn_off() { gpio_instance->turn_off(); }

    void toggle() { gpio_instance->toggle(); }
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
