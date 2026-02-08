/*
 * LookupSensor.hpp
 *
 *  Created on: Nov 7, 2022
 *      Author: ricardo
 */

#pragma once
#include <cstdint>
#include "HALAL/Services/ADC/NewADC.hpp"
#include "Sensors/AnalogUtils.hpp"
#include "ErrorHandler/ErrorHandler.hpp"
#include "C++Utilities/CppUtils.hpp"

#define REFERENCE_VOLTAGE 3.3


class LookupSensor{
public:
	LookupSensor() = default;
	LookupSensor(ST_LIB::ADCDomain::Instance& adc, double *table, int table_size, double *value);
	LookupSensor(ST_LIB::ADCDomain::Instance& adc, double *table, int table_size, double &value);
	void read();

protected:
	ST_LIB::ADCDomain::Instance* adc = nullptr;
	double *table = nullptr;
	int table_size = 0;
	double *value = nullptr;
};
