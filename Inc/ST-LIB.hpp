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

template <typename Tuple> struct UnpackToCtx;
template <typename... Ds> struct UnpackToCtx<std::tuple<Ds...>> {
  using type = BuildCtx<Ds...>;
};

template <auto &...devs> struct Board {
  using Domains = std::tuple<
                            /* Level 0: Base Domains */
                            GPIODomain,
                            /* Level 1: Domains depending on Level 0 */
                            DigitalOutputDomain, DigitalInputDomain /*, ADCDomain, PWMDomain, ...*/>;
  using CtxType = typename UnpackToCtx<Domains>::type;

  static consteval auto build_ctx() {
    CtxType ctx{};
    (devs.inscribe(ctx), ...);
    return ctx;
  }

  static constexpr auto ctx = build_ctx();

  template <typename D> static consteval std::size_t domain_size() {
    return ctx.template span<D>().size();
  }

  template <size_t... Is>
  static consteval auto build_configs(std::index_sequence<Is...>) {
    return std::make_tuple(
        std::tuple_element_t<Is, Domains>::template build<
            domain_size<std::tuple_element_t<Is, Domains>>()>(
            ctx.template span<std::tuple_element_t<Is, Domains>>())...);
  }

  static consteval auto build() {
    return build_configs(
        std::make_index_sequence<std::tuple_size_v<Domains>>{});
  }

  static constexpr auto cfg = build();

  template <typename Domain> static consteval auto get_config() {
    constexpr std::size_t I = CtxType::template domain_index<Domain>();
    return std::get<I>(cfg);
  }

  template <typename Domain> static auto &get_instances() {
    constexpr std::size_t N = domain_size<Domain>();
    return Domain::template Init<N, get_config<Domain>()>::instances;
  }

  template <typename InitT, typename... Deps>
  static void init_with_deps(std::tuple<Deps...> *) { // Pointer to deduce types easily, not used
    InitT::init(get_instances<Deps>()...);
  }

  template <typename Domain> static void init_domain() {
    constexpr std::size_t N = domain_size<Domain>();
    constexpr auto cfgs = get_config<Domain>();
    using InitT = typename Domain::template Init<N, cfgs>;

    if constexpr (requires { typename Domain::dependencies; }) {
      // Resolve dependencies
      init_with_deps<InitT>(static_cast<typename Domain::dependencies *>(nullptr));
    } else {
      InitT::init();
    }
  }

  template <std::size_t... Is>
  static void init_domains(std::index_sequence<Is...>) {
    (init_domain<std::tuple_element_t<Is, Domains>>(), ...);
  }

  static void init() {
    init_domains(std::make_index_sequence<std::tuple_size_v<Domains>>{});
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

    return Domain::template Init<N, get_config<Domain>()>::instances[idx];
  }
};

} // namespace ST_LIB