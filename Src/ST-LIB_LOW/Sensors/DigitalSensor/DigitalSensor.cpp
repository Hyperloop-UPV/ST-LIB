#include "Sensors/DigitalSensor/DigitalSensor.hpp"

#include "HALAL/Services/DigitalInputService/DigitalInputService.hpp"

DigitalSensor::DigitalSensor(Pin& pin, PinState* value)
    : id(DigitalInput::inscribe(pin)), value(value) {}

DigitalSensor::DigitalSensor(Pin& pin, PinState& value)
    : DigitalSensor::DigitalSensor(pin, &value) {}

void DigitalSensor::read() {
    PinState val = DigitalInput::read_pin_state(id);

    *DigitalSensor::value = val;
}

uint8_t DigitalSensor::get_id() { return id; }
