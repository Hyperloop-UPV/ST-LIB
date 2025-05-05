/*
 * EthernetNode.cpp
 *
 *  Created on: Nov 23, 2022
 *      Author: stefa
 */

#pragma once

#include "C++Utilities/CppUtils.hpp"
#include "HALAL/Models/PinModel/Pin.hpp"
#include "HALAL/Services/Communication/Ethernet/EthernetNode.hpp"
#include "lwip.h"
#include "ethernetif.h"
#include "HALAL/Services/Communication/Ethernet/EthernetHelper.hpp"
#include "HALAL/Models/MAC/MAC.hpp"

#ifdef HAL_ETH_MODULE_ENABLED

#define ETHERNET_POOLS_BASE_ADDRESS 0x30000000

#define TCP_INACTIVITY_TIME_UNTIL_KEEPALIVE_MS 500
#define TCP_KEEPALIVE_TRIES_UNTIL_DISCONNECTION 10
#define TCP_SPACE_BETWEEN_KEEPALIVE_TRIES_MS 100

class Ethernet{
public:
	static bool is_ready;
	static bool is_running;

	static void inscribe();
	static void start(string local_mac, string local_ip, string subnet_mask, string gateway);
	static void start(MAC local_mac, IPV4 local_ip, IPV4 subnet_mask, IPV4 gateway);

	/**
	 * @brief handles the received messages by ethernet
	 */
	static void update();

private:
	static void mpu_start();
};

#endif
