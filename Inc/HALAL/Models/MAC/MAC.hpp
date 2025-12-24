#pragma once

#include <iomanip>

#include "C++Utilities/CppUtils.hpp"
#include "lwip/init.h"
#include "lwip/ip_addr.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "netif.h"
#include "netif/etharp.h"
#include "netif/ethernet.h"
#include "stm32h7xx_hal.h"
#include "timeouts.h"

#ifdef HAL_ETH_MODULE_ENABLED

class MAC {
   public:
    uint8_t address[6];
    // string string_address;

    MAC();
    // MAC(string address);
    MAC(uint8_t address[6]);
    
    static MAC parse_string(const char* address);
};

#endif
