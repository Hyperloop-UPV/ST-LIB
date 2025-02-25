/*
 * LookupSensor.hpp
 *
 *  Created on: Nov 7, 2022
 *      Author: ricardo
 */

#pragma once
#include "HALAL/HALAL.hpp"
#include "ErrorHandler/ErrorHandler.hpp"
#include "C++Utilities/CppUtils.hpp"

#define REFERENCE_VOLTAGE 3.3


class LookupSensor{
public:
	LookupSensor() = default;
	LookupSensor(Pin &pin, double *table, int table_size, double *value);
	LookupSensor(Pin &pin, double *table, int table_size, double &value);
	void read();
	uint8_t get_id();

protected:
	uint8_t id;
	double *table;
	int table_size;
	double *value;
};
