/*
 * SNTP.hpp
 *
 *  Created on: 21 feb. 2023
 *      Author: Ricardo
 */

#pragma once

#include "sntp.h"
#include "HALAL/Models/IPV4/IPV4.hpp"
#include "HALAL/Services/Time/Time.hpp"
#include "C++Utilities/CppUtils.hpp"

class SNTP {
public:
	static void sntp_update(uint8_t address_head, uint8_t address_second, uint8_t address_third, uint8_t address_last);
	static void sntp_update(string ip);
	static void sntp_update();

};
