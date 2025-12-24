#pragma once

#include <string>

#include "HALAL/HALAL.hpp"
// #include "ST-LIB_HIGH.hpp"
// #include "ST-LIB_LOW.hpp"

class STLIB {
public:
#ifdef STLIB_ETH
  static void start(MAC mac, IPV4 ip, IPV4 subnet_mask, IPV4 gateway
                    );

//   static void start(const std::string &mac = "00:80:e1:00:00:00",
//                     const std::string &ip = "192.168.1.4",
//                     const std::string &subnet_mask = "255.255.0.0",
//                     const std::string &gateway = "192.168.1.1");
#else
  static void start(UART::Peripheral &printf_peripheral = UART::uart2);
#endif

  static void update();
};