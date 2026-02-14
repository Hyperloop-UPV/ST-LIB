#include "HALAL/Services/Communication/Ethernet/PHY/phy_driver.h"
#include "stm32h7xx_hal.h"

#ifdef USE_PHY_KSZ8041

extern ETH_HandleTypeDef heth;
#ifndef KSZ8041_PHY_ADDRESS
#define KSZ8041_PHY_ADDRESS 1
#endif

static uint32_t phy_read(uint32_t reg) {
    uint32_t val;
    HAL_ETH_ReadPHYRegister(&heth, KSZ8041_PHY_ADDRESS, reg, &val);
    return val;
}

static phy_link_state_t ksz8041_get_link_state(void) {
    static uint8_t link_latched = 0;

    uint32_t bmsr;
    HAL_ETH_ReadPHYRegister(&heth, KSZ8041_PHY_ADDRESS, 1, &bmsr);
    HAL_ETH_ReadPHYRegister(&heth, KSZ8041_PHY_ADDRESS, 1, &bmsr);

    if (!(bmsr & (1 << 2))) {
        link_latched = 0;
        return PHY_LINK_DOWN;
    }

    if (!link_latched) {
        link_latched = 1;
        return PHY_100_FULL;
    }

    return PHY_100_FULL;
}

const phy_driver_t phy_ksz8041 = {.init = NULL, .get_link_state = ksz8041_get_link_state};

#endif
