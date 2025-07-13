#include "Sensors/DigitalSensor/DigitalSensor.hpp"
#include "Sensors/Sensor/Sensor.hpp"


DigitalSensor::DigitalSensor(Pin &pin, PinState *value) : id(DigitalInput::inscribe(pin)), value(value){}

DigitalSensor::DigitalSensor(Pin &pin, PinState &value) : DigitalSensor::DigitalSensor(pin,&value){}

DigitalSensor::DigitalSensor(Pin &pin, PinState *value, OperationMode mode) : id(DigitalInput::inscribe(pin, mode)), value(value){}

DigitalSensor::DigitalSensor(Pin &pin, PinState &value, OperationMode mode) : DigitalSensor::DigitalSensor(pin,&value, mode){}

void DigitalSensor::read(){
	PinState val = DigitalInput::read_pin_state(id);

	*DigitalSensor::value = val;
}

uint8_t DigitalSensor::get_id(){
	return id;
}
