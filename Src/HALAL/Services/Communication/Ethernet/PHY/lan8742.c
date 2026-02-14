#include "HALAL/Services/Communication/Ethernet/PHY/phy_driver.h"

#ifdef USE_PHY_LAN8742

#include "lan8742.h"
#include "stm32h7xx_hal.h"

extern ETH_HandleTypeDef heth;

/* Bus IO provided by ethernetif.c */
extern int32_t ETH_PHY_IO_Init(void);
extern int32_t ETH_PHY_IO_DeInit(void);
extern int32_t ETH_PHY_IO_WriteReg(uint32_t DevAddr, uint32_t RegAddr, uint32_t RegVal);
extern int32_t ETH_PHY_IO_ReadReg(uint32_t DevAddr, uint32_t RegAddr, uint32_t* pRegVal);
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
    uint32_t id1 = 0, id2 = 0;

    HAL_ETH_SetMDIOClockRange(&heth);
    LAN8742_RegisterBusIO(&lan, &ctx);

    if (lan.IO.Init) {
        lan.IO.Init();
    }

    lan.Is_Initialized = 0;
    lan.DevAddr = LAN8742_PHY_ADDRESS;

    /* Prefer configured address; fall back to MDIO scan if not responsive */
    if (lan.IO.ReadReg(lan.DevAddr, LAN8742_PHYI1R, &id1) < 0 ||
        lan.IO.ReadReg(lan.DevAddr, LAN8742_PHYI2R, &id2) < 0 || (id1 == 0x0000U) ||
        (id1 == 0xFFFFU)) {
        for (uint32_t addr = 0; addr <= 31; ++addr) {
            if (lan.IO.ReadReg(addr, LAN8742_PHYI1R, &id1) < 0) {
                continue;
            }
            if (lan.IO.ReadReg(addr, LAN8742_PHYI2R, &id2) < 0) {
                continue;
            }
            if ((id1 != 0x0000U) && (id1 != 0xFFFFU)) {
                lan.DevAddr = addr;
                break;
            }
        }
    }

    if ((id1 != 0x0000U) && (id1 != 0xFFFFU)) {
        lan.Is_Initialized = 1;
    }

    if (lan.Is_Initialized) {
        LAN8742_DisablePowerDownMode(&lan);
        LAN8742_StartAutoNego(&lan);

        uint32_t bcr = 0;
        if (lan.IO.ReadReg(lan.DevAddr, LAN8742_BCR, &bcr) >= 0) {
            bcr |= (LAN8742_BCR_AUTONEGO_EN | LAN8742_BCR_RESTART_AUTONEGO);
            lan.IO.WriteReg(lan.DevAddr, LAN8742_BCR, bcr);
        }
    }
}

static phy_link_state_t lan_get_link_state(void) {
    if (!lan.Is_Initialized) {
        return PHY_LINK_DOWN;
    }
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
