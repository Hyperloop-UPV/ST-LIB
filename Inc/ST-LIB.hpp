#pragma once

#include "HALAL/HALAL.hpp" // Erase this when ST-LIB_LOW is finished.
#include "ST-LIB_LOW.hpp"  // Erase this when ST-LIB_HIGH is finished
#include "ST-LIB_HIGH.hpp" // Highest layer up to the moment
class STLIB {
public:

	static void start(MAC mac, IPV4 ip, IPV4 subnet_mask, IPV4 gateaway, UART::Peripheral& printf_peripheral);
	static void start(
			string mac = "00:80:e1:00:00:00",
			string ip = "192.168.1.4",
			string subnet_mask = "255.255.0.0",
			string gateway = "192.168.1.1",
			UART::Peripheral& printf_peripheral = UART::uart2);

	static void update();
};
