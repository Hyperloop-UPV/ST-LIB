#include "HALAL/Services/Communication/Ethernet/PHY/phy_driver.h"

#ifdef USE_PHY_LAN8700

#include "stm32h7xx_hal.h"

extern ETH_HandleTypeDef heth;

#ifndef LAN8700_PHY_ADDRESS
#define LAN8700_PHY_ADDRESS 31
#endif

#define LAN8700_BCR 0x00U
#define LAN8700_BSR 0x01U
#define LAN8700_PHYID1 0x02U
#define LAN8700_PHYID2 0x03U
#define LAN8700_ANAR 0x04U
#define LAN8700_ANLPAR 0x05U
#define LAN8700_SMR 0x12U
#define LAN8700_PHYSCSR 0x1FU

#define LAN8700_BCR_RESET 0x8000U
#define LAN8700_BCR_AUTONEG 0x1000U
#define LAN8700_BCR_RESTART_AUTONEG 0x0200U
#define LAN8700_BCR_POWER_DOWN 0x0800U
#define LAN8700_BCR_SPEED_SELECT 0x2000U
#define LAN8700_BCR_DUPLEX_MODE 0x0100U

#define LAN8700_BSR_LINK_STATUS 0x0004U
#define LAN8700_BSR_AUTONEG_COMPLETE 0x0020U

#define LAN8700_ANAR_100FD 0x0100U
#define LAN8700_ANAR_100HD 0x0080U
#define LAN8700_ANAR_10FD  0x0040U
#define LAN8700_ANAR_10HD  0x0020U


static uint32_t lan8700_active_addr = LAN8700_PHY_ADDRESS;

static uint32_t lan8700_read(uint32_t reg) {
  uint32_t val = 0;
  HAL_ETH_ReadPHYRegister(&heth, lan8700_active_addr, reg, &val);
  return val;
}

static void lan8700_write(uint32_t reg, uint32_t val) {
  HAL_ETH_WritePHYRegister(&heth, lan8700_active_addr, reg, val);
}

static void lan8700_wait_reset_clear(void) {
  uint32_t start = HAL_GetTick();
  while (HAL_GetTick() - start < 100U) {
    if ((lan8700_read(LAN8700_BCR) & LAN8700_BCR_RESET) == 0U) {
      break;
    }
  }
}

static uint8_t lan8700_scan_address(void) {
  uint32_t id1 = 0U;
  uint32_t id2 = 0U;
  for (uint32_t addr = 0; addr <= 31; ++addr) {
    if (HAL_ETH_ReadPHYRegister(&heth, addr, LAN8700_PHYID1, &id1) != HAL_OK) {
      continue;
    }
    if (HAL_ETH_ReadPHYRegister(&heth, addr, LAN8700_PHYID2, &id2) != HAL_OK) {
      continue;
    }
    if ((id1 != 0x0000U) && (id1 != 0xFFFFU)) {
      return (uint8_t)addr;
    }
  }
  return 0xFF;
}

static void lan8700_init(void) {
  HAL_ETH_SetMDIOClockRange(&heth);

  /* Verify MDIO responds at configured address, otherwise auto-scan */
  uint32_t id1 = 0U;
  if (HAL_ETH_ReadPHYRegister(&heth, lan8700_active_addr, LAN8700_PHYID1,
                              &id1) != HAL_OK ||
      (id1 == 0x0000U) || (id1 == 0xFFFFU)) {
    uint8_t addr = lan8700_scan_address();
    if (addr == 0xFF) {
      return;
    }
    lan8700_active_addr = addr;
  }

  uint32_t bcr = lan8700_read(LAN8700_BCR);
  if (bcr & LAN8700_BCR_POWER_DOWN) {
    bcr &= ~LAN8700_BCR_POWER_DOWN;
    lan8700_write(LAN8700_BCR, bcr);
  }

  /* Soft reset to load strap defaults */
  lan8700_write(LAN8700_BCR, LAN8700_BCR_RESET);
  lan8700_wait_reset_clear();

  /* Start auto-negotiation (or force 100FD for debug) */
  bcr = lan8700_read(LAN8700_BCR);
  bcr |= LAN8700_BCR_AUTONEG | LAN8700_BCR_RESTART_AUTONEG;
  lan8700_write(LAN8700_BCR, bcr);
}

static phy_link_state_t lan8700_get_link_state(void) {
  uint32_t bsr = lan8700_read(LAN8700_BSR);
  /* BSR is latch-low: read twice to clear latched bits */
  bsr = lan8700_read(LAN8700_BSR);

  if ((bsr & LAN8700_BSR_LINK_STATUS) == 0U) {
    return PHY_LINK_DOWN;
  }

  uint32_t bcr = lan8700_read(LAN8700_BCR);
  if ((bcr & LAN8700_BCR_AUTONEG) == 0U) {
    const uint32_t speed_100 = (bcr & LAN8700_BCR_SPEED_SELECT);
    const uint32_t full = (bcr & LAN8700_BCR_DUPLEX_MODE);
    if (speed_100) {
      return full ? PHY_100_FULL : PHY_100_HALF;
    }
    return full ? PHY_10_FULL : PHY_10_HALF;
  }

  if ((bsr & LAN8700_BSR_AUTONEG_COMPLETE) == 0U) {
    return PHY_LINK_DOWN;
  }

  uint32_t anar = lan8700_read(LAN8700_ANAR);
  uint32_t anlpar = lan8700_read(LAN8700_ANLPAR);
  uint32_t common = anar & anlpar;

  if (common & LAN8700_ANAR_100FD) {
    return PHY_100_FULL;
  }
  if (common & LAN8700_ANAR_100HD) {
    return PHY_100_HALF;
  }
  if (common & LAN8700_ANAR_10FD) {
    return PHY_10_FULL;
  }
  if (common & LAN8700_ANAR_10HD) {
    return PHY_10_HALF;
  }

  return PHY_LINK_DOWN;
}

const phy_driver_t phy_lan8700 = {
    .init = lan8700_init,
    .get_link_state = lan8700_get_link_state,
};

#endif /* USE_PHY_LAN8700 */
