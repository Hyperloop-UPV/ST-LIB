#include "ST-LIB.hpp"

#ifdef STLIB_ETH

// Con Ethernet: interfaz con MAC/IP + overload con strings
void STLIB::start(MAC mac, IPV4 ip, IPV4 subnet_mask, IPV4 gateway
                  ) {
  HALAL::start(mac, ip, subnet_mask, gateway);
  // STLIB_LOW::start();
  // STLIB_HIGH::start();
}

// void STLIB::start(const std::string &mac, const std::string &ip,
//                   const std::string &subnet_mask, const std::string &gateway
//                   ) {
//   STLIB::start(MAC(mac), IPV4(ip), IPV4(subnet_mask), IPV4(gateway)
//                );
// }

#else // !STLIB_ETH

void STLIB::start(UART::Peripheral &printf_peripheral) {
  HALAL::start(printf_peripheral);
  STLIB_LOW::start();
  STLIB_HIGH::start();
}

#endif // STLIB_ETH

void STLIB::update() {
#ifdef NDEBUG
#ifdef HAL_IWDG_MODULE_ENABLED
  Watchdog::refresh();
#endif
#endif

#if !defined STLIB_ETH
#else
  Ethernet::update();
  // Server::update_servers();
#endif

  ErrorHandlerModel::ErrorHandlerUpdate();
}