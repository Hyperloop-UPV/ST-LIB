#include "HALAL/Services/Communication/Ethernet/PHY/phy_driver.h"

#if defined(USE_PHY_KSZ8041)
extern const phy_driver_t phy_ksz8041;
const phy_driver_t* phy_driver = &phy_ksz8041;

#elif defined(USE_PHY_LAN8742)
extern const phy_driver_t phy_lan8742;
const phy_driver_t* phy_driver = &phy_lan8742;

#elif defined(USE_PHY_LAN8700)
extern const phy_driver_t phy_lan8700;
const phy_driver_t* phy_driver = &phy_lan8700;

#else
#error "No PHY selected"
#endif
