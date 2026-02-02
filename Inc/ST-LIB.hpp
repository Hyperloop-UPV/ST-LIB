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

/* DomainXList params:
 *  - First param: Domain name
 *  - Second param: instance name
 *  - Third param: true if it needs the nº of instances to be templated,
 *                 false otherwise (mainly for MPUdomain)
 *  - Rest: any dependencies on previous domains
 */
#define DomainXList \
    X(MPUDomain, mpu, true) NEXT                \
    X(GPIODomain, gpio, false) NEXT             \
    X(TimerDomain, tim, false) NEXT             \
    X(DigitalOutputDomain, dout, false, GPIODomain::Init<gpioN>::instances) NEXT \
    X(DigitalInputDomain, din, false, GPIODomain::Init<gpioN>::instances) NEXT \
    X(MdmaPacketDomain, mdmaPacket, false, MPUDomain::Init<mpuN, cfg.mpu_cfgs>::instances) NEXT \
    /* X(ADCDomain, adc, false) NEXT */         \
    X(SdDomain, sd, false, MPUDomain::Init<mpuN, cfg.mpu_cfgs>::instances, DigitalInputDomain::Init<dinN>::instances)

#define NEXT ,
#define X(domain, inst, is_templated, ...) domain

using DomainCtx = BuildCtx<DomainXList>;

#undef NEXT
#undef X

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
#define NEXT ;
#define X(domain, inst, is_templated, ...) \
    constexpr std::size_t inst##N = domain_size<domain>();
    
    DomainXList;

#undef NEXT
#undef X

    struct ConfigBundle {
#define NEXT ;
#define X(domain, inst, is_templated, ...) \
    std::array<domain::Config, inst##N> inst##_cfgs
    
      DomainXList;

#undef NEXT
#undef X
    };

    return ConfigBundle{
#define NEXT ,
#define X(domain, inst, is_templated, ...) \
    .inst##_cfgs = domain::template build<inst##N>(ctx.template span<domain>())

        DomainXList,

#undef NEXT
#undef X
    };
  }

  static constexpr auto cfg = build();

  static void init() {
#define NEXT ;
#define X(domain, inst, is_templated, ...) \
    constexpr std::size_t inst##N = domain_size<domain>();
    
    DomainXList;

#undef X

#define X(domain, inst, is_templated, ...) \
    if constexpr(!is_templated) { \
        domain::Init<inst##N>::init(cfg.inst##_cfgs, ##__VA_ARGS__); \
    }

    /* Any templated go here since I couldn't get it to compile otherwise */
    MPUDomain::Init<mpuN, cfg.mpu_cfgs>::init();
    DomainXList;

#undef NEXT
#undef X
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
