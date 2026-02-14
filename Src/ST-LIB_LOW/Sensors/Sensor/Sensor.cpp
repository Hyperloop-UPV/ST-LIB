#include "Sensors/Sensor/Sensor.hpp"

#include "HALAL/Services/InputCapture/InputCapture.hpp"

std::vector<uint8_t> Sensor::inputcapture_id_list{};

void Sensor::start() {

    for (uint8_t inputcapture_id : inputcapture_id_list) {
        InputCapture::turn_on(inputcapture_id);
    }
}
