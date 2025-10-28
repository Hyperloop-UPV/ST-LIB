#include "Sensors/LookupSensor/LookupSensor.hpp"
#include "Sensors/Sensor/Sensor.hpp"
#ifdef LOOKUP_SENSOR_

LookupSensor::LookupSensor(Pin &pin, double *table, int table_size, double *value) : table(table), table_size(table_size), value(value){
	id = ADC::inscribe(pin);

	Sensor::adc_id_list.insert(Sensor::adc_id_list.begin(),id);
}

LookupSensor::LookupSensor(Pin &pin, double *table, int table_size, double &value) : LookupSensor::LookupSensor(pin,table,table_size,&value){}

void LookupSensor::read(){
	float adc_voltage = ADC::get_value(id);

	int table_index = (int)(adc_voltage * table_size / REFERENCE_VOLTAGE);
	if(table_index >= table_size){
		table_index = table_size - 1;
	}
	*value = table[table_index];
}

uint8_t LookupSensor::get_id(){
	return id;
}
#endif