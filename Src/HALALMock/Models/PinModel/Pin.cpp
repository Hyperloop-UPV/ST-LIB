/*
 * Pin.cpp
 *
 *  Created on: 19 oct. 2022
 *      Author: stefan
 */

#include "HALALMock/Models/PinModel/Pin.hpp"
#include "ErrorHandler/ErrorHandler.hpp"

Pin::Pin(){}

Pin::Pin(GPIOPort port, GPIOPin gpio_pin) : port((GPIO_TypeDef*)port), gpio_pin(gpio_pin){}

Pin::Pin(GPIOPort port, GPIOPin gpio_pin, AlternativeFunction alternative_function) :
		port((GPIO_TypeDef*)port),
		gpio_pin(gpio_pin),
		alternative_function(alternative_function)
		{}

const vector<reference_wrapper<Pin>> Pin::pinVector = {PA0,PA1,PA10,PA11,PA12,
PA9,PB0,PB1,PB10,PB11,PB12,PB13,PB14,PB15,PB2,PB4,PB5,PB6,PB7,PB8,PB9,PC0,PC1,PC10,
PC11,PC12,PC13,PC14,PC15,PC2,PC3,PC4,PC5,PC6,PC7,PC8,PC9,PD0,PD1,PD10,PD11,PD12,PD13,
PD14,PD15,PD2,PD3,PD4,PD5,PD6,PD7,PD8,PD9,PE0,PE1,PE10,PE11,PE12,PE13,PE14,PE15,PE2,PE3,
PE4,PE5,PE6,PE7,PE8,PE9,PF0,PF1,PF10,PF11,PF12,PF13,PF14,PF15,PF2,PF3,PF4,PF5,PF6,PF7,
PF8,PF9,PG0,PG1,PG10,PG11,PG12,PG13,PG14,PG15,PG2,PG3,PG4,PG5,PG6,PG7,PG8,PG9,PH0,PH1,
PA2,PA3,PA4,PA5,PA6,PA7,PA8};

const map<GPIOPin,const string> Pin::gpio_pin_to_string = {{PIN_0,"0"}, {PIN_1,"1"}, {PIN_2,"2"}, {PIN_3,"3"}, {PIN_4,"4"}, {PIN_5,"5"}, {PIN_6,"6"}, {PIN_7,"7"}, {PIN_8,"8"}, {PIN_9,"9"}, {PIN_10,"10"}, {PIN_11,"11"}, {PIN_12,"12"}, {PIN_13,"13"}, {PIN_14,"14"}, {PIN_15,"15"},{PIN_ALL,"ALL"}};
const map<GPIO_TypeDef*,const string> Pin::port_to_string = {{(GPIO_TypeDef*)PORT_A,"PA"}, {(GPIO_TypeDef*)PORT_B,"PB"}, {(GPIO_TypeDef*)PORT_C,"PC"}, {(GPIO_TypeDef*)PORT_D,"PD"}, {(GPIO_TypeDef*)PORT_E,"PE"}, {(GPIO_TypeDef*)PORT_F,"PF"}, {(GPIO_TypeDef*)PORT_G,"PG"}, {(GPIO_TypeDef*)PORT_H,"PH"}};

const string Pin::to_string() const {
	return (port_to_string.at(port) + gpio_pin_to_string.at(gpio_pin));
}

void Pin::inscribe(Pin& pin, OperationMode mode){
	if(pin.mode != OperationMode::NOT_USED){
		ErrorHandler("Pin %s is already registered, cannot register twice", pin.to_string().c_str());
		return;
	}
	pin.mode = mode;
}

void Pin::start(){}
