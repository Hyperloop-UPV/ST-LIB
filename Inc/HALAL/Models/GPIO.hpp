#pragma once

#include "stm32h7xx_hal.h"
#include <array>
#include <span>
#include <tuple>

using std::array;
using std::size_t;
using std::span;
using std::tuple;

namespace ST_LIB {
struct GPIODomain {
  enum class OperationMode : uint8_t {
    INPUT,
    OUTPUT,
    ANALOG,
    EXTERNAL_INTERRUPT_RISING,
    EXTERNAL_INTERRUPT_FALLING,
    EXTERNAL_INTERRUPT_RISING_FALLING,
    TIMER_ALTERNATE_FUNCTION,
    ALTERNATIVE,
  };
  enum class Port : uint8_t { A, B, C, D, E, F, G, H };
  static inline GPIO_TypeDef *port_to_reg(Port p) {
    switch (p) {
    case Port::A:
      return GPIOA;
    case Port::B:
      return GPIOB;
    case Port::C:
      return GPIOC;
    case Port::D:
      return GPIOD;
    case Port::E:
      return GPIOE;
    case Port::F:
      return GPIOF;
    case Port::G:
      return GPIOG;
    case Port::H:
      return GPIOH;
    default:
      return nullptr;
    }
  }
  static inline void enable_gpio_clock(Port port) {
    switch (port) {
    case Port::A:
      __HAL_RCC_GPIOA_CLK_ENABLE();
      break;

    case Port::B:
      __HAL_RCC_GPIOB_CLK_ENABLE();
      break;

    case Port::C:
      __HAL_RCC_GPIOC_CLK_ENABLE();
      break;

    case Port::D:
      __HAL_RCC_GPIOD_CLK_ENABLE();
      break;

    case Port::E:
      __HAL_RCC_GPIOE_CLK_ENABLE();
      break;

    case Port::F:
      __HAL_RCC_GPIOF_CLK_ENABLE();
      break;

    case Port::G:
      __HAL_RCC_GPIOG_CLK_ENABLE();
      break;

    case Port::H:
      __HAL_RCC_GPIOH_CLK_ENABLE();
      break;
    }
  }

  struct Pin2 {
    GPIODomain::Port port;
    uint32_t pin;

    consteval Pin2(GPIODomain::Port port, uint32_t pin)
        : port(port), pin(pin) {}
  };

  struct Entry {
    Port port;
    uint32_t pin;
    OperationMode mode;
  };

  struct GPIO {
    using domain = GPIODomain;

    Entry e;

    consteval GPIO(Pin2 pin, OperationMode mode) : e(pin.port, pin.pin, mode) {}

    template <class Ctx> consteval void inscribe(Ctx &ctx) const {
      ctx.template add<GPIODomain>(e);
    }
  };

  static constexpr std::size_t max_instances{110};
  static_assert(max_instances > 0,
                "The number of instances must be greater than 0");

  struct Config {
    std::tuple<Port, GPIO_InitTypeDef> init_data{};
  };

  template <size_t N>
  static consteval array<Config, N> build(span<const Entry> pins) {
    array<Config, N> cfgs{};
    for (std::size_t i = 0; i < N; ++i) {
      const auto &e = pins[i];

      for (std::size_t j = 0; j < i; ++j) {
        const auto &prev = pins[j];
        if (prev.pin == e.pin && prev.port == e.port) {
          struct gpio_already_inscribed {};
          throw gpio_already_inscribed{};
        }
      }

      GPIO_InitTypeDef GPIO_InitStruct;
      GPIO_InitStruct.Pin = e.pin;
      switch (e.mode) {

      case OperationMode::OUTPUT:
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        break;

      case OperationMode::INPUT:
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        break;

      case OperationMode::ANALOG:
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        break;
      case OperationMode::EXTERNAL_INTERRUPT_RISING:
        GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
        GPIO_InitStruct.Pull = GPIO_PULLDOWN;
        break;
      case OperationMode::EXTERNAL_INTERRUPT_FALLING:
        GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
        GPIO_InitStruct.Pull = GPIO_PULLDOWN;
        break;
      case OperationMode::EXTERNAL_INTERRUPT_RISING_FALLING:
        GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
        GPIO_InitStruct.Pull = GPIO_PULLDOWN;
        break;
        //   case OperationMode::TIMER_ALTERNATE_FUNCTION:
        //     GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        //     GPIO_InitStruct.Pull = GPIO_NOPULL;
        //     GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        //     GPIO_InitStruct.Alternate = pin.alternative_function;
        //     break;

      default:
        break;
      }

      cfgs[i].init_data = std::make_tuple(e.port, GPIO_InitStruct);
    }

    return cfgs;
  }

  // Runtime object
  struct Instance {
    GPIO_TypeDef *port;
    uint16_t pin;

    void turn_on() { HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET); }

    void turn_off() { HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET); }

    void toggle() { HAL_GPIO_TogglePin(port, pin); }
  };

private:
  inline static Instance *instances_ptr = nullptr;

public:
  static Instance &instance(std::size_t id) { return instances_ptr[id]; }

  template <std::size_t N> struct Init {
    static inline std::array<Instance, N> instances{};

    static void init(std::span<const Config, N> cfgs) {
      static_assert(N > 0);
      for (std::size_t i = 0; i < N; ++i) {
        const auto &e = cfgs[i];
        auto [port, gpio_init] = e.init_data;

        enable_gpio_clock(port);
        HAL_GPIO_Init(port_to_reg(port), &gpio_init);

        auto &inst = instances[i];
        inst.port = port_to_reg(port);
        inst.pin = gpio_init.Pin;
      }

      instances_ptr = instances.data();
    }
  };
};
} // namespace ST_LIB