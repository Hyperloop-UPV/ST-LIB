/*
 * EthernetNode.cpp
 *
 *  Created on: Nov 23, 2022
 *      Author: stefa
 */

#pragma once

#include "C++Utilities/CppUtils.hpp"
#include "HALALMock/Models/PinModel/Pin.hpp"
#include "HALALMock/Services/Communication/Ethernet/EthernetNode.hpp"
#include "HALALMock/Services/Communication/Ethernet/EthernetHelper.hpp"
//changed the times for linux
#define TCP_INACTIVITY_TIME_UNTIL_KEEPALIVE 1
#define TCP_KEEPALIVE_TRIES_UNTIL_DISCONNECTION 10
#define TCP_SPACE_BETWEEN_KEEPALIVE_TRIES 1

class Ethernet{
public:
	static bool is_ready;
	static bool is_running;

	static void inscribe();
	static void start(string local_ip, string subnet_mask, string gateway);
	static void start(IPV4 local_ip, IPV4 subnet_mask, IPV4 gateway);

	/**
	 * @brief handles the received messages by ethernet
	 */
	static void update();

private:
	static void mpu_start();
};
