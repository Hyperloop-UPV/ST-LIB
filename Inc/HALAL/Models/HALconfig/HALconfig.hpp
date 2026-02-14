/*
 * HALconfig.hpp
 *
 *  Created on: 5 ene. 2023
 *      Author: aleja
 */

#pragma once

#include "stm32h7xx_hal.h"
#include "ErrorHandler/ErrorHandler.hpp"

enum TARGET { Nucleo, Board };

namespace HALconfig {
void system_clock();
void peripheral_clock();

} // namespace HALconfig
