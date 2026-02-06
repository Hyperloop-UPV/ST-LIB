#include "Sensors/Sensor/Sensor.hpp"

#include "HALAL/Services/ADC/ADC.hpp"
#include "HALAL/Services/EXTI/EXTI.hpp"
#include "HALAL/Services/InputCapture/InputCapture.hpp"

std::vector<uint8_t> Sensor::adc_id_list{};
std::vector<uint8_t> Sensor::EXTI_id_list{};
std::vector<uint8_t> Sensor::inputcapture_id_list{};

void Sensor::start(){
	for(uint8_t adc_id : adc_id_list){
		ADC::turn_on(adc_id);
	}

	for(uint8_t exti_id : EXTI_id_list){
		ExternalInterrupt::turn_on(exti_id);
	}

	for(uint8_t inputcapture_id : inputcapture_id_list){
		InputCapture::turn_on(inputcapture_id);
	}

}
