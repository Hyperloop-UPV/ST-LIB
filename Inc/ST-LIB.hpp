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

using DomainsCtx = BuildCtx<GPIODomain, DigitalOutputDomain,
                            DigitalInputDomain, DMA_Domain, SPIDomain /*, ADCDomain, PWMDomain, ...*/>;

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
    constexpr std::size_t gpioN = domain_size<GPIODomain>();
    constexpr std::size_t doutN = domain_size<DigitalOutputDomain>();
    constexpr std::size_t dinN = domain_size<DigitalInputDomain>();
    constexpr std::size_t dmaN = domain_size<DMA_Domain>();
    constexpr std::size_t spiN = domain_size<SPIDomain>();
    // ...

    struct ConfigBundle {
      std::array<GPIODomain::Config, gpioN> gpio_cfgs;
      std::array<DigitalOutputDomain::Config, doutN> dout_cfgs;
      std::array<DigitalInputDomain::Config, dinN> din_cfgs;
      std::array<DMA_Domain::Config, dmaN> dma_cfgs;
      std::array<SPIDomain::Config, spiN> spi_cfgs;
      // ...
    };

    return ConfigBundle{
        .gpio_cfgs =
            GPIODomain::template build<gpioN>(ctx.template span<GPIODomain>()),
        .dout_cfgs = DigitalOutputDomain::template build<doutN>(
            ctx.template span<DigitalOutputDomain>()),
        .din_cfgs = DigitalInputDomain::template build<dinN>(
            ctx.template span<DigitalInputDomain>()),
        .dma_cfgs = DMA_Domain::template build<dmaN>(
            ctx.template span<DMA_Domain>()),
        .spi_cfgs = SPIDomain::template build<spiN>(
            ctx.template span<SPIDomain>()),
        // ...
    };
  }

  static constexpr auto cfg = build();

  static void init() {
    constexpr std::size_t gpioN = domain_size<GPIODomain>();
    constexpr std::size_t doutN = domain_size<DigitalOutputDomain>();
    constexpr std::size_t dinN = domain_size<DigitalInputDomain>();
    constexpr std::size_t dmaN = domain_size<DMA_Domain>();
    constexpr std::size_t spiN = domain_size<SPIDomain>();
    // ...

    GPIODomain::Init<gpioN>::init(cfg.gpio_cfgs);
    DigitalOutputDomain::Init<doutN>::init(cfg.dout_cfgs,
                                           GPIODomain::Init<gpioN>::instances);
    DigitalInputDomain::Init<dinN>::init(cfg.din_cfgs,
                                         GPIODomain::Init<gpioN>::instances);
    DMA_Domain::Init<dmaN>::init(cfg.dma_cfgs);
    SPIDomain::Init<spiN>::init(cfg.spi_cfgs,
                                GPIODomain::Init<gpioN>::instances,
                                DMA_Domain::Init<dmaN>::instances);
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
    return Domain::template Init<N>::instances[idx];
  }
};

} // namespace ST_LIB