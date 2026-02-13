#include "Sensors/SensorInterrupt/SensorInterrupt.hpp"


SensorInterrupt::SensorInterrupt(ST_LIB::EXTIDomain::Instance &exti, GPIO_PinState *value) : exti(&exti), value(value) {
}

SensorInterrupt::SensorInterrupt(ST_LIB::EXTIDomain::Instance &exti, GPIO_PinState &value) : SensorInterrupt::SensorInterrupt(exti, &value){}

void SensorInterrupt::read(){
	*value = exti->read();
}

