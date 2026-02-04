#include "HALAL/Services/Communication/Ethernet/PHY/phy_driver.h"

#ifdef USE_PHY_LAN8742

#include "lan8742.h"
#include "stm32h7xx_hal.h"

/* Bus IO provided by ethernetif.c */
extern int32_t ETH_PHY_IO_Init(void);
extern int32_t ETH_PHY_IO_DeInit(void);
extern int32_t ETH_PHY_IO_WriteReg(uint32_t DevAddr, uint32_t RegAddr,
                                   uint32_t RegVal);
extern int32_t ETH_PHY_IO_ReadReg(uint32_t DevAddr, uint32_t RegAddr,
                                  uint32_t *pRegVal);
extern int32_t ETH_PHY_IO_GetTick(void);

static lan8742_Object_t lan;

static lan8742_IOCtx_t ctx = {
    .Init = ETH_PHY_IO_Init,
    .DeInit = ETH_PHY_IO_DeInit,
    .WriteReg = ETH_PHY_IO_WriteReg,
    .ReadReg = ETH_PHY_IO_ReadReg,
    .GetTick = ETH_PHY_IO_GetTick,
};

static void lan_init(void) {
  /* Explicit PHY address (robust) */
  lan.DevAddr = LAN8742_PHY_ADDRESS;

  LAN8742_RegisterBusIO(&lan, &ctx);
  LAN8742_Init(&lan);
}

static phy_link_state_t lan_get_link_state(void) {
  int32_t st = LAN8742_GetLinkState(&lan);

  switch (st) {
  case LAN8742_STATUS_100MBITS_FULLDUPLEX:
    return PHY_100_FULL;
  case LAN8742_STATUS_100MBITS_HALFDUPLEX:
    return PHY_100_HALF;
  case LAN8742_STATUS_10MBITS_FULLDUPLEX:
    return PHY_10_FULL;
  case LAN8742_STATUS_10MBITS_HALFDUPLEX:
    return PHY_10_HALF;
  default:
    return PHY_LINK_DOWN;
  }
}

const phy_driver_t phy_lan8742 = {
    .init = lan_init,
    .get_link_state = lan_get_link_state,
};

#endif /* USE_PHY_LAN8742 */