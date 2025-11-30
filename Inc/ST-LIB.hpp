#pragma once

#include <string>

#include "HALAL/HALAL.hpp"
#include "ST-LIB_HIGH.hpp"
#include "ST-LIB_LOW.hpp"

class STLIB {
public:
#ifdef STLIB_ETH
  static void start(MAC mac, IPV4 ip, IPV4 subnet_mask, IPV4 gateway,
                    UART::Peripheral &printf_peripheral);

  static void start(const std::string &mac = "00:80:e1:00:00:00",
                    const std::string &ip = "192.168.1.4",
                    const std::string &subnet_mask = "255.255.0.0",
                    const std::string &gateway = "192.168.1.1",
                    UART::Peripheral &printf_peripheral = UART::uart2);
#else
  static void start(UART::Peripheral &printf_peripheral = UART::uart2);
#endif

  static void update();
};

namespace ST_LIB {
template <typename... Domains> struct BuildCtx {
  template <typename D> using Decl = typename D::Entry;
  template <typename D>
  static constexpr std::size_t max_count_v = D::max_instances;

  std::tuple<std::array<Decl<Domains>, max_count_v<Domains>>...> storage{};
  std::array<std::size_t, sizeof...(Domains)> sizes{};

  template <typename D, std::size_t I = 0>
  static consteval std::size_t domain_index() {
    if constexpr (I >= sizeof...(Domains)) {
      static_assert([] { return false; }(), "Domain not found");
      return 0;
    } else if constexpr (std::is_same_v<D, std::tuple_element_t<
                                               I, std::tuple<Domains...>>>) {
      return I;
    } else {
      return domain_index<D, I + 1>();
    }
  }

  template <typename D> consteval void add(typename D::Entry e) {
    constexpr std::size_t I = domain_index<D>();
    auto &arr = std::get<I>(storage);
    auto &size = sizes[I];
    arr[size++] = e;
  }

  template <typename D> consteval auto span() const {
    constexpr std::size_t I = domain_index<D>();
    auto const &arr = std::get<I>(storage);
    auto const size = sizes[I];
    using E = typename D::Entry;
    return std::span<const E>{arr.data(), size};
  }
};

template <typename Domain, auto... devs> consteval std::size_t domain_count() {
  return (... +
          (std::is_same_v<typename decltype(devs)::domain, Domain> ? 1u : 0u));
}

using DomainsCtx = BuildCtx<GPIODomain /*, ADCDomain, PWMDomain, ...*/>;

// Configure HW (compile-time)
template <auto... devs> consteval auto build() {
  DomainsCtx ctx{};

  (devs.inscribe(ctx), ...);

  constexpr std::size_t gpioN = domain_count<GPIODomain, devs...>();
  // constexpr std::size_t adcN = domain_count<ADCDomain, devs...>();
  // constexpr std::size_t pwmN = domain_count<PWMDomain, devs...>();

  struct ConfigBundle {
    array<GPIODomain::Config, gpioN> gpio_cfgs;
    // array<ADCDomain::Config, adcN> adc_cgfs;
    // array<PWMDomain::Config, pwmN> pwm_cgfs;
  };

  return ConfigBundle{
      .gpio_cfgs =
          GPIODomain::template build<gpioN>(ctx.template span<GPIODomain>()),
      // .adc_cgfs =
      //     ADCDomain::template build<adcN>(ctx.template span<ADCDomain>()),
      // .pwm_cgfs =
      //     PWMDomain::template build<pwmN>(ctx.template span<PWMDomain>()),
  };
}

// Init real HW (runtime)
template <auto... devs> void init() {
  static constexpr auto cfg = build<devs...>();

  constexpr std::size_t gpioN = domain_count<GPIODomain, devs...>();
  // constexpr std::size_t adcN = domain_count<ADCDomain, devs...>();
  // constexpr std::size_t pwmN = domain_count<PWMDomain, devs...>();

  GPIODomain::Init<gpioN>::init(cfg.gpio_cfgs);
  // ADCDomain::Init<adcN>::init(cfg.adc_cfgs);
  // PWMDomain::Init<pwmN>::init(cfg.pwm_cfgs);
}

} // namespace ST_LIB