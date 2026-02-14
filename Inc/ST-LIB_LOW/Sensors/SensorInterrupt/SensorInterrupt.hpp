
/*
 * SensorInterrupt.hpp
 *
 *  Created on: Jan 31, 2023
 *      Author: ricardo
 */

#pragma once
#include <cstdint>
#include <functional>

#include "HALAL/Services/EXTI/EXTI.hpp"

class SensorInterrupt {
public:
    SensorInterrupt() = default;
    SensorInterrupt(ST_LIB::EXTIDomain::Instance& exti, GPIO_PinState* value);
    SensorInterrupt(ST_LIB::EXTIDomain::Instance& exti, GPIO_PinState& value);
    void read();

protected:
    ST_LIB::EXTIDomain::Instance* exti;
    GPIO_PinState* value;
};
