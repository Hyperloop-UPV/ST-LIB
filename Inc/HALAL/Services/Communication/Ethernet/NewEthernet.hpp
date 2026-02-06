#pragma once

#include "stm32h7xx_hal.h"

#include "C++Utilities/CppUtils.hpp"
#include "DigitalOutput2.hpp"

#ifdef STLIB_ETH
#include "HALAL/Models/MAC/MAC.hpp"
#include "HALAL/Services/Communication/Ethernet/LWIP/EthernetHelper.hpp"
#include "HALAL/Services/Communication/Ethernet/LWIP/EthernetNode.hpp"
extern "C" {
#include "ethernetif.h"
#include "lwip.h"
}

extern uint32_t EthernetLinkTimer;
extern struct netif gnetif;
extern ip4_addr_t ipaddr, netmask, gw;
extern uint8_t IP_ADDRESS[4], NETMASK_ADDRESS[4], GATEWAY_ADDRESS[4];

namespace ST_LIB {
extern void compile_error(const char *msg);

#ifndef PHY_RESET_LOW_DELAY_MS
#define PHY_RESET_LOW_DELAY_MS 10
#endif
#ifndef PHY_RESET_HIGH_DELAY_MS
#define PHY_RESET_HIGH_DELAY_MS 10
#endif

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
    const GPIODomain::Pin &PHY_RST;
  };

  constexpr static EthernetPins PINSET_H10{.MDC = PC1,
                                           .REF_CLK = PA1,
                                           .MDIO = PA2,
                                           .CRS_DV = PA7,
                                           .RXD0 = PC4,
                                           .RXD1 = PC5,
                                           .TXD1 = PB13,
                                           .TX_EN = PG11,
                                           .TXD0 = PG13,
                                           .PHY_RST = PG0};
  constexpr static EthernetPins PINSET_H11{.MDC = PC1,
                                           .REF_CLK = PA1,
                                           .MDIO = PA2,
                                           .CRS_DV = PA7,
                                           .RXD0 = PC4,
                                           .RXD1 = PC5,
                                           .TXD1 = PB13,
                                           .TX_EN = PB11,
                                           .TXD0 = PB12,
                                           .PHY_RST = PF14};

  struct Entry {
    const char *local_mac;
    const char *local_ip;
    const char *subnet_mask;
    const char *gateway;

    size_t phy_reset_id;
  };

  struct Ethernet {
    using domain = EthernetDomain;

    EthernetPins pins;
    Entry e;

    std::array<GPIODomain::GPIO, 9> rmii_gpios;
    DigitalOutputDomain::DigitalOutput phy_reset;

    consteval Ethernet(EthernetPins pins, const char *local_mac,
                       const char *local_ip,
                       const char *subnet_mask = "255.255.0.0",
                       const char *gateway = "192.168.1.1")
        : pins{pins}, e{local_mac, local_ip, subnet_mask, gateway},
          rmii_gpios{
              GPIODomain::GPIO(pins.MDC, GPIODomain::OperationMode::ALT_PP,
                               GPIODomain::Pull::None,
                               GPIODomain::Speed::VeryHigh,
                               GPIODomain::AlternateFunction::AF11),
              GPIODomain::GPIO(pins.REF_CLK, GPIODomain::OperationMode::ALT_PP,
                               GPIODomain::Pull::None,
                               GPIODomain::Speed::VeryHigh,
                               GPIODomain::AlternateFunction::AF11),
              GPIODomain::GPIO(pins.MDIO, GPIODomain::OperationMode::ALT_OD,
                               GPIODomain::Pull::Up,
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
                               GPIODomain::AlternateFunction::AF11)},
          phy_reset{pins.PHY_RST} {}

    template <class Ctx> consteval std::size_t inscribe(Ctx &ctx) const {
      for (const auto &gpio : rmii_gpios) {
        gpio.inscribe(ctx);
      }

      const auto phy_reset_id = phy_reset.inscribe(ctx);
      Entry entry{
          .local_mac = this->e.local_mac,
          .local_ip = this->e.local_ip,
          .subnet_mask = this->e.subnet_mask,
          .gateway = this->e.gateway,
          .phy_reset_id = phy_reset_id,
      };

      return ctx.template add<EthernetDomain>(entry, this);
    }
  };

  static constexpr std::size_t max_instances{1};

  struct Config {
    const char *local_mac;
    const char *local_ip;
    const char *subnet_mask;
    const char *gateway;

    size_t phy_reset_id;
  };

  template <size_t N>
  static consteval array<Config, N> build(span<const Entry> config) {
    array<Config, N> cfgs{};
    static_assert(N == max_instances,
                  "EthernetDomain only supports a single instance");
    const auto &e = config[0];
    cfgs[0].local_mac = e.local_mac;
    cfgs[0].local_ip = e.local_ip;
    cfgs[0].subnet_mask = e.subnet_mask;
    cfgs[0].gateway = e.gateway;
    cfgs[0].phy_reset_id = e.phy_reset_id;

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

        if (netif_is_link_up(&gnetif) && !netif_is_up(&gnetif)) {
          netif_set_up(&gnetif);
        }
      }
    };
  };

  template <std::size_t N> struct Init {
    static inline std::array<Instance, N> instances{};

    static void init(std::span<const Config, N> cfgs,
                     std::span<DigitalOutputDomain::Instance> do_instances) {
      static_assert(N == max_instances,
                    "EthernetDomain only supports a single instance");
      const EthernetDomain::Config &e = cfgs[0];

      /* --- RESET PHY --- */
      // RESET_N pin low then high
      do_instances[e.phy_reset_id].turn_off(); // RESET_N = 0
      HAL_Delay(PHY_RESET_LOW_DELAY_MS);
      do_instances[e.phy_reset_id].turn_on(); // RESET_N = 1
      HAL_Delay(PHY_RESET_HIGH_DELAY_MS);

      /* --- CLOCKS ETH --- */
      __HAL_RCC_ETH1MAC_CLK_ENABLE();
      __HAL_RCC_ETH1TX_CLK_ENABLE();
      __HAL_RCC_ETH1RX_CLK_ENABLE();

      /* --- NVIC --- */
      HAL_NVIC_SetPriority(ETH_IRQn, 0, 0);
      HAL_NVIC_EnableIRQ(ETH_IRQn);

      /* --- IP / MAC --- */
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
      gnetif.hwaddr_len = 6;

      /* --- LwIP / ETH init --- */
      MX_LWIP_Init();

      instances[0] = Instance{};
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
    static void init(std::span<const Config, N> cfgs,
                     std::span<DigitalOutputDomain::Instance> do_instances) {};
  };
};
} // namespace ST_LIB
#endif // STLIB_ETH
