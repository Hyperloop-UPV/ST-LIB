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
extern void compile_error(const char *msg);
template <typename... Domains> struct BuildCtx {
  template <typename D> using Decl = typename D::Entry;
  template <typename D>
  static constexpr std::size_t max_count_v = D::max_instances;

  std::tuple<std::array<Decl<Domains>, max_count_v<Domains>>...> storage{};
  std::tuple<std::array<const void *, max_count_v<Domains>>...> owners{};
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

  template <typename D, typename Owner>
  consteval std::size_t add(typename D::Entry e, const Owner *owner) {
    constexpr std::size_t I = domain_index<D>();
    auto &arr = std::get<I>(storage);
    auto &own = std::get<I>(owners);
    auto &size = sizes[I];

    const auto idx = size;
    arr[size] = e;
    own[size] = owner;
    ++size;
    return idx;
  }

  template <typename D> consteval auto span() const {
    constexpr std::size_t I = domain_index<D>();
    auto const &arr = std::get<I>(storage);
    auto const size = sizes[I];
    using E = typename D::Entry;
    return std::span<const E>{arr.data(), size};
  }

  template <typename D> consteval auto owners_span() const {
    constexpr std::size_t I = domain_index<D>();
    auto const &arr = std::get<I>(owners);
    auto const size = sizes[I];
    return std::span<const void *const>{arr.data(), size};
  }

  template <typename D> consteval std::size_t size() const {
    constexpr std::size_t I = domain_index<D>();
    return sizes[I];
  }
};

using DomainsCtx = BuildCtx<MPUDomain, GPIODomain, TimerDomain,
                            DigitalOutputDomain,
                            DigitalInputDomain,
                            MdmaPacketDomain,
                            SdDomain, /*, ADCDomain, PWMDomain, ...*/>;

template <auto &...devs> struct Board {
  static consteval auto build_ctx() {
    DomainsCtx ctx{};
    (devs.inscribe(ctx), ...);
    return ctx;
  }

  static constexpr auto ctx = build_ctx();

  template <typename D> static consteval std::size_t domain_size() {
    return ctx.template span<D>().size();
  }

  static consteval auto build() {
    constexpr std::size_t mpuN = domain_size<MPUDomain>();
    constexpr std::size_t gpioN = domain_size<GPIODomain>();
    constexpr std::size_t timN = domain_size<TimerDomain>();
    constexpr std::size_t doutN = domain_size<DigitalOutputDomain>();
    constexpr std::size_t dinN = domain_size<DigitalInputDomain>();
    constexpr std::size_t mdmaPacketN = domain_size<MdmaPacketDomain>();
    constexpr std::size_t sdN = domain_size<SdDomain>();
    // ...

    struct ConfigBundle {
      std::array<MPUDomain::Config, mpuN> mpu_cfgs;
      std::array<GPIODomain::Config, gpioN> gpio_cfgs;
      std::array<TimerDomain::Config, timN> tim_cfgs;
      std::array<DigitalOutputDomain::Config, doutN> dout_cfgs;
      std::array<DigitalInputDomain::Config, dinN> din_cfgs;
      std::array<MdmaPacketDomain::Config, mdmaPacketN> mdma_packet_cfgs;
      std::array<SdDomain::Config, sdN> sd_cfgs;
      // ...
    };

    return ConfigBundle{
        .mpu_cfgs = MPUDomain::template build<mpuN>(
            ctx.template span<MPUDomain>()),
        .gpio_cfgs =
            GPIODomain::template build<gpioN>(ctx.template span<GPIODomain>()),
        .tim_cfgs =
            TimerDomain::template build<timN>(ctx.template span<TimerDomain>()),
        .dout_cfgs = DigitalOutputDomain::template build<doutN>(
            ctx.template span<DigitalOutputDomain>()),
        .din_cfgs = DigitalInputDomain::template build<dinN>(
            ctx.template span<DigitalInputDomain>()),
        .mdma_packet_cfgs = MdmaPacketDomain::template build<mdmaPacketN>(
            ctx.template span<MdmaPacketDomain>())
        .sd_cfgs = SdDomain::template build<sdN>(
            ctx.template span<SdDomain>()),
        // ...
    };
  }

  static constexpr auto cfg = build();

  static void init() {
    constexpr std::size_t mpuN = domain_size<MPUDomain>();
    constexpr std::size_t gpioN = domain_size<GPIODomain>();
    constexpr std::size_t timN = domain_size<TimerDomain>();
    constexpr std::size_t doutN = domain_size<DigitalOutputDomain>();
    constexpr std::size_t dinN = domain_size<DigitalInputDomain>();
    constexpr std::size_t mdmaPacketN = domain_size<MdmaPacketDomain>();
    constexpr std::size_t sdN = domain_size<SdDomain>();
    // ...

    MPUDomain::Init<mpuN, cfg.mpu_cfgs>::init();
    GPIODomain::Init<gpioN>::init(cfg.gpio_cfgs);
    TimerDomain::Init<timN>::init(cfg.tim_cfgs);
    DigitalOutputDomain::Init<doutN>::init(cfg.dout_cfgs,
                                           GPIODomain::Init<gpioN>::instances);
    DigitalInputDomain::Init<dinN>::init(cfg.din_cfgs,
                                         GPIODomain::Init<gpioN>::instances);
    MdmaPacketDomain::Init<mdmaPacketN>::init(cfg.mdma_packet_cfgs,
                                              MPUDomain::Init<mpuN, cfg.mpu_cfgs>::instances);
    SdDomain::Init<sdN>::init(cfg.sd_cfgs,
                              MPUDomain::Init<mpuN, cfg.mpu_cfgs>::instances,
                              DigitalInputDomain::Init<dinN>::instances);
    // ...
  }

  template <typename Domain, auto &Target, std::size_t I = 0>
  static consteval std::size_t owner_index_of() {
    constexpr auto owners = ctx.template owners_span<Domain>();

    if constexpr (I >= owners.size()) {
      compile_error("Device not registered in domain");
      return 0;
    } else {
      return owners[I] == &Target ? I : owner_index_of<Domain, Target, I + 1>();
    }
  }

  template <auto &Target> static auto &instance_of() {
    using DevT = std::remove_cvref_t<decltype(Target)>;
    using Domain = typename DevT::domain;

    constexpr std::size_t idx = owner_index_of<Domain, Target>();

    constexpr std::size_t N = domain_size<Domain>();
    
    if constexpr (std::is_same_v<Domain, MPUDomain>) {
      return Domain::template Init<N, cfg.mpu_cfgs>::instances[idx];
    } else {
      return Domain::template Init<N>::instances[idx];
    }
  }
};

} // namespace ST_LIB