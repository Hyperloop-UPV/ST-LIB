#include "Sensors/SensorInterrupt/SensorInterrupt.hpp"
#include "Sensors/Sensor/Sensor.hpp"
#ifdef SENSOR_
SensorInterrupt::SensorInterrupt(Pin &pin, std::function<void()> &&action, PinState *value, TRIGGER trigger) : value(value) {
	id = ExternalInterrupt::inscribe(pin, std::forward<std::function<void()>>(action), trigger);

	Sensor::EXTI_id_list.insert(Sensor::EXTI_id_list.begin(),id);
}

SensorInterrupt::SensorInterrupt(Pin &pin, std::function<void()> &&action, PinState &value, TRIGGER trigger) : SensorInterrupt::SensorInterrupt(pin,std::forward<std::function<void()>>(action),&value, trigger){}

void SensorInterrupt::read(){
	*value = (PinState)ExternalInterrupt::get_pin_value(id);
}

uint8_t SensorInterrupt::get_id(){
	return id;
}
#endif