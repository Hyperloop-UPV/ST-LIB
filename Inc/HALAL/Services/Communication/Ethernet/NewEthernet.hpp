#pragma once

#include "stm32h7xx_hal.h"

#include "C++Utilities/CppUtils.hpp"

#ifdef STLIB_ETH
#include "HALAL/Models/MAC/MAC.hpp"
#include "HALAL/Services/Communication/Ethernet/EthernetHelper.hpp"
#include "HALAL/Services/Communication/Ethernet/EthernetNode.hpp"
#include "ethernetif.h"
#include "lwip.h"

extern uint32_t EthernetLinkTimer;
extern struct netif gnetif;
extern ip4_addr_t ipaddr, netmask, gw;
extern uint8_t IP_ADDRESS[4], NETMASK_ADDRESS[4], GATEWAY_ADDRESS[4];

namespace ST_LIB {
extern void compile_error(const char *msg);

struct EthernetDomain {

  struct EthernetPins {
    const GPIODomain::Pin &MDC;
    const GPIODomain::Pin &REF_CLK;
    const GPIODomain::Pin &MDIO;
    const GPIODomain::Pin &CRS_DV;
    const GPIODomain::Pin &RXD0;
    const GPIODomain::Pin &RXD1;
    const GPIODomain::Pin &TXD1;
    const GPIODomain::Pin &TX_EN;
    const GPIODomain::Pin &TXD0;
  };

  constexpr static EthernetPins PINSET_H10{.MDC = PC1,
                                           .REF_CLK = PA1,
                                           .MDIO = PA2,
                                           .CRS_DV = PA7,
                                           .RXD0 = PC4,
                                           .RXD1 = PC5,
                                           .TXD1 = PB13,
                                           .TX_EN = PG11,
                                           .TXD0 = PG13};
  constexpr static EthernetPins PINSET_H11{.MDC = PC1,
                                           .REF_CLK = PA1,
                                           .MDIO = PA2,
                                           .CRS_DV = PA7,
                                           .RXD0 = PC4,
                                           .RXD1 = PC5,
                                           .TXD1 = PB13,
                                           .TX_EN = PB11,
                                           .TXD0 = PB12};

  struct Entry {
    const char *local_mac;
    const char *local_ip;
    const char *subnet_mask;
    const char *gateway;
  };

  struct Ethernet {
    using domain = EthernetDomain;

    EthernetPins pins;
    Entry e;

    std::array<GPIODomain::GPIO, 9> gpios;

    consteval Ethernet(EthernetPins pins, const char *local_mac,
                       const char *local_ip,
                       const char *subnet_mask = "255.255.0.0",
                       const char *gateway = "192.168.1.1")
        : pins{pins}, e{local_mac, local_ip, subnet_mask, gateway},
          gpios{
              GPIODomain::GPIO(pins.MDC, GPIODomain::OperationMode::ALT_PP,
                               GPIODomain::Pull::None,
                               GPIODomain::Speed::VeryHigh,
                               GPIODomain::AlternateFunction::AF11),
              GPIODomain::GPIO(pins.REF_CLK, GPIODomain::OperationMode::ALT_PP,
                               GPIODomain::Pull::None,
                               GPIODomain::Speed::VeryHigh,
                               GPIODomain::AlternateFunction::AF11),
              GPIODomain::GPIO(pins.MDIO, GPIODomain::OperationMode::ALT_PP,
                               GPIODomain::Pull::None,
                               GPIODomain::Speed::VeryHigh,
                               GPIODomain::AlternateFunction::AF11),
              GPIODomain::GPIO(pins.CRS_DV, GPIODomain::OperationMode::ALT_PP,
                               GPIODomain::Pull::None,
                               GPIODomain::Speed::VeryHigh,
                               GPIODomain::AlternateFunction::AF11),
              GPIODomain::GPIO(pins.RXD0, GPIODomain::OperationMode::ALT_PP,
                               GPIODomain::Pull::None,
                               GPIODomain::Speed::VeryHigh,
                               GPIODomain::AlternateFunction::AF11),
              GPIODomain::GPIO(pins.RXD1, GPIODomain::OperationMode::ALT_PP,
                               GPIODomain::Pull::None,
                               GPIODomain::Speed::VeryHigh,
                               GPIODomain::AlternateFunction::AF11),
              GPIODomain::GPIO(pins.TXD1, GPIODomain::OperationMode::ALT_PP,
                               GPIODomain::Pull::None,
                               GPIODomain::Speed::VeryHigh,
                               GPIODomain::AlternateFunction::AF11),
              GPIODomain::GPIO(pins.TX_EN, GPIODomain::OperationMode::ALT_PP,
                               GPIODomain::Pull::None,
                               GPIODomain::Speed::VeryHigh,
                               GPIODomain::AlternateFunction::AF11),
              GPIODomain::GPIO(pins.TXD0, GPIODomain::OperationMode::ALT_PP,
                               GPIODomain::Pull::None,
                               GPIODomain::Speed::VeryHigh,
                               GPIODomain::AlternateFunction::AF11),
          } {}

    template <class Ctx> consteval std::size_t inscribe(Ctx &ctx) const {
      for (const auto &gpio : gpios) {
        gpio.inscribe(ctx);
      }

      return ctx.template add<EthernetDomain>(e, this);
    }
  };

  static constexpr std::size_t max_instances{1};

  struct Config {
    const char *local_mac;
    const char *local_ip;
    const char *subnet_mask;
    const char *gateway;
  };

  template <size_t N>
  static consteval array<Config, N> build(span<const Entry> config) {
    array<Config, N> cfgs{};
    for (std::size_t i = 0; i < N; ++i) {
      const auto &e = config[i];
      cfgs[i].local_mac = e.local_mac;
      cfgs[i].local_ip = e.local_ip;
      cfgs[i].subnet_mask = e.subnet_mask;
      cfgs[i].gateway = e.gateway;
    }

    return cfgs;
  }

  // Runtime object
  struct Instance {
    constexpr Instance() {}
    void update() {
      ethernetif_input(&gnetif);
      sys_check_timeouts();

      if (HAL_GetTick() - EthernetLinkTimer >= 100) {
        EthernetLinkTimer = HAL_GetTick();
        ethernet_link_check_state(&gnetif);

        if (gnetif.flags == 15) {
          netif_set_up(&gnetif);
        }
      }
    };
  };

  template <std::size_t N> struct Init {
    static inline std::array<Instance, N> instances{};

    static void init(std::span<const Config, N> cfgs) {
      for (std::size_t i = 0; i < N; ++i) {
        const EthernetDomain::Config &e = cfgs[i];

        __HAL_RCC_ETH1MAC_CLK_ENABLE();
        __HAL_RCC_ETH1TX_CLK_ENABLE();
        __HAL_RCC_ETH1RX_CLK_ENABLE();

        HAL_NVIC_SetPriority(ETH_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(ETH_IRQn);

        MAC local_mac{e.local_mac};
        IPV4 local_ip{e.local_ip};
        IPV4 subnet_mask{e.subnet_mask};
        IPV4 gateway{e.gateway};

        ipaddr = local_ip.address;
        netmask = subnet_mask.address;
        gw = gateway.address;
        IP_ADDRESS[0] = ipaddr.addr & 0xFF;
        IP_ADDRESS[1] = (ipaddr.addr >> 8) & 0xFF;
        IP_ADDRESS[2] = (ipaddr.addr >> 16) & 0xFF;
        IP_ADDRESS[3] = (ipaddr.addr >> 24) & 0xFF;
        NETMASK_ADDRESS[0] = netmask.addr & 0xFF;
        NETMASK_ADDRESS[1] = (netmask.addr >> 8) & 0xFF;
        NETMASK_ADDRESS[2] = (netmask.addr >> 16) & 0xFF;
        NETMASK_ADDRESS[3] = (netmask.addr >> 24) & 0xFF;
        GATEWAY_ADDRESS[0] = gw.addr & 0xFF;
        GATEWAY_ADDRESS[1] = (gw.addr >> 8) & 0xFF;
        GATEWAY_ADDRESS[2] = (gw.addr >> 16) & 0xFF;
        GATEWAY_ADDRESS[3] = (gw.addr >> 24) & 0xFF;
        gnetif.hwaddr[0] = local_mac.address[0];
        gnetif.hwaddr[1] = local_mac.address[1];
        gnetif.hwaddr[2] = local_mac.address[2];
        gnetif.hwaddr[3] = local_mac.address[3];
        gnetif.hwaddr[4] = local_mac.address[4];
        gnetif.hwaddr[5] = local_mac.address[5];
        MX_LWIP_Init();

        instances[i] = Instance{};
      }
    }
  };
};
} // namespace ST_LIB

#else
namespace ST_LIB {
// Dummy EthernetDomain when STLIB_ETH is not defined
struct EthernetDomain {
  static constexpr std::size_t max_instances{0};
  struct Entry {};
  struct Config {};
  template <size_t N>
  static consteval array<Config, N> build(span<const Entry> config) {
    return {};
  }
  struct Instance {};
  template <std::size_t N> struct Init {
    static void init(std::span<const Config, N> cfgs) {}
  };
};
} // namespace ST_LIB
#endif // STLIB_ETH