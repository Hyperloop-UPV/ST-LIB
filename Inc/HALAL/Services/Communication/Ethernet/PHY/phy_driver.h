#pragma once
#include <stdint.h>

typedef enum {
    PHY_LINK_DOWN = 0,
    PHY_10_HALF,
    PHY_10_FULL,
    PHY_100_HALF,
    PHY_100_FULL
} phy_link_state_t;

typedef struct {
    void (*init)(void);
    phy_link_state_t (*get_link_state)(void);
} phy_driver_t;

extern const phy_driver_t* phy_driver;
