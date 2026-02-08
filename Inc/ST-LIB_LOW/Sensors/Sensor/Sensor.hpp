/*
 * Sensor.hpp
 *
 *  Created on: Nov 2, 2022
 *      Author: ricardo
 */

#pragma once
#include <cstdint>
#include <vector>

class Sensor{
public:
	static void start();
	static std::vector<uint8_t> EXTI_id_list;
	static std::vector<uint8_t> inputcapture_id_list;
};
