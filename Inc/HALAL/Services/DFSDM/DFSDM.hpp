#pragma once

#include "HALAL/Models/GPIO.hpp"
using ST_LIB::GPIODomain;
/*
Not added extremes detector
It doesn't make sense with the use that we have.

This table lists all the DFSDM interrupt sources:
• End of conversion events, with separate flags for
regular and injected conversions.
• Data overrun events, with separate flags for regular
and injected conversions.
• Analog watchdog events.
• Short-circuit detector events.
• Channel clock absence event.Interrupts:

*/

namespace ST_LIB {
struct DFSDMDomain{
    //Recommend to use Run please.
    enum class Modes: uint8_t{
        Run, //Active
        Sleep, //Active. Peripheral interrupts cause the device to exit Sleep Mode
        Low_pw_run, //Active but low power
        Low_pw_sleep, //Sleep + low power
        Stop, //Frozen, peripheral register content is kept
        Standby,//Powered-down, the peripheral must be reinitialized after exiting Standby Mode
        Shutdown //Powered-down. Same as Standby
    };
    enum class Filters: uint8_t{
        Sinc1,
        Sinc2,
        Sinc3,
        Sinc4,
        Sinc5,
        FastSinc
    };
    enum class Use_Pin: uint8_t{
        Clock_Out,
        Serial_Channel
    };
    //only expected to use SPI-Like mode
    //20MHz max  = max 80 Mhz sys clock divider by 4
    enum class SPI_Mode: uint8_t{
        Falling,
        Rising
    };
    enum class Data_Write: uint8_t{
        DMA,
        CPU
    };
    enum class Analog_Watchdog: uint8_t{
        Active,
        Inactive
    };
    //What happens when data exceed with watchdog
    enum class Watchdog_Action: uint8_t {
        interrupt,
        Break_signal
    }
    //what data is monitoring the watchdog
    enum class Watchdog_Type: uint8_t {
        WD1,
        WD2,
        WD3
    };
    enum class Short_circuit: uint8_t{
        Active,
        Inactive
    };
    enum class Conversion_Type: uint8_t{
        Regular, //Less priority and can be interrupted by injected, in that case starts again
        Injected 
    };
    enum class Regular_mode{
        Single,
        Continuous
    };
    enum class Injected_mode: uint8_t{
        scan, //all channels from the injected mode are converted when a trigger occurs
        single //only one channel is converted
    };
 struct Entry {
    size_t gpio_idx;
    uint16_t oversampling;
    uint8_t integrator;
    uint32_t offset;
    uint32_t right_bit_shifting;
    uint32_t high_threshold;
    uint32_t low_threshold;
    uint32_t detection_signal_saturated; // Number of 1s or 0s to detect configurable time
    void* short_circuit_handler;
  };
  struct DigitalInput {
    GPIODomain::GPIO gpio;
    using domain = DFSDMDomain;

    consteval DFSDM(const GPIODomain::Pin &pin,)
        : gpio{pin, GPIODomain::OperationMode::INPUT, pull, speed} {}

    template <class Ctx> consteval std::size_t inscribe(Ctx &ctx) const {
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

//Sinc1,2,3 -> oversampling from 1 to 32
