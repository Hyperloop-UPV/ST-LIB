#pragma once
#include "stm32h7xx_hal.h"

#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "lwip/ip_addr.h"
#include "lwip/init.h"
#include "netif.h"
#include "netif/ethernet.h"
#include "netif/etharp.h"
#include "timeouts.h"
#include "C++Utilities/CppUtils.hpp"

#ifdef HAL_ETH_MODULE_ENABLED

using std::getline;
using std::stringstream;

class IPV4 {
public:
    ip_addr_t address;
    string string_address;

    IPV4();
    IPV4(const char* address);
    IPV4(string address);
    IPV4(ip_addr_t address);

    void operator=(const char* address);
};

#endif
