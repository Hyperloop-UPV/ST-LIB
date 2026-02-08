#include "Sensors/LookupSensor/LookupSensor.hpp"


LookupSensor::LookupSensor(ST_LIB::ADCDomain::Instance& adc, double *table, int table_size, double *value)
	: adc(&adc), table(table), table_size(table_size), value(value){}

LookupSensor::LookupSensor(ST_LIB::ADCDomain::Instance& adc, double *table, int table_size, double &value)
	: LookupSensor::LookupSensor(adc, table, table_size, &value){}

void LookupSensor::read(){
	if (adc == nullptr || value == nullptr) {
		return;
	}
	const uint32_t raw = adc->read_raw();
	const float adc_voltage = ST_LIB::adc_raw_to_voltage(raw, adc->resolution);

	int table_index = (int)(adc_voltage * table_size / REFERENCE_VOLTAGE);
	if(table_index >= table_size){
		table_index = table_size - 1;
	}
	*value = table[table_index];
}
