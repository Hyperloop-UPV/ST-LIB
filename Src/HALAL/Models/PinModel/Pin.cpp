/*
 * Pin.cpp
 *
 *  Created on: 19 oct. 2022
 *      Author: stefan
 */

#include "HALAL/Models/PinModel/Pin.hpp"
#include "ErrorHandler/ErrorHandler.hpp"



// consteval std::string_view Pin::port_string() const {
//     switch (*port) {
//         case PORT_A: return "PA";
//         case PORT_B: return "PB";
//         case PORT_C: return "PC";
//         case PORT_D: return "PD";
//         case PORT_E: return "PE";
// 		case PORT_F: return "PF";
//         case PORT_G: return "PG";
//         case PORT_H: return "PH";
//     }
// 	static_assert(always_false<GPIO_TypeDef*>, "Pin::port_string(): puerto no valido");
// }
// consteval std::string_view Pin::pin_string() const {
//     switch(gpio_pin){
//         case PIN_0: return "0";   case PIN_1: return "1";   case PIN_2: return "2";
//         case PIN_3: return "3";   case PIN_4: return "4";   case PIN_5: return "5";
//         case PIN_6: return "6";   case PIN_7: return "7";   case PIN_8: return "8";
//     	case PIN_9: return "9";   case PIN_10: return "10"; case PIN_11: return "11";
//         case PIN_12: return "12"; case PIN_13: return "13"; case PIN_14: return "14";
//         case PIN_15: return "15"; case PIN_ALL: return "ALL";
//     }
//     static_assert(always_false<GPIOPin>, "Pin::pin_string(): Pin no válido");
// }

// consteval std::string_view Pin::to_string() const {
//         constexpr size_t max_len = 5;
//         char buffer[max_len] = {}; 
//         size_t idx = 0;
//         for (char c : port_string()) buffer[idx++] = c;
//         for (char c : pin_string())  buffer[idx++] = c;
//         return std::string_view(buffer, idx);
// }
// const map<GPIOPin,const string> gpio_pin_to_string = {{PIN_0,"0"}, {PIN_1,"1"}, {PIN_2,"2"}, {PIN_3,"3"}, {PIN_4,"4"}, {PIN_5,"5"}, {PIN_6,"6"}, {PIN_7,"7"}, {PIN_8,"8"}, {PIN_9,"9"}, {PIN_10,"10"}, {PIN_11,"11"}, {PIN_12,"12"}, {PIN_13,"13"}, {PIN_14,"14"}, {PIN_15,"15"},{PIN_ALL,"ALL"}};
// const map<GPIO_TypeDef*,const string> port_to_string = {{(GPIO_TypeDef*)PORT_A,"PA"}, {(GPIO_TypeDef*)PORT_B,"PB"}, {(GPIO_TypeDef*)PORT_C,"PC"}, {(GPIO_TypeDef*)PORT_D,"PD"}, {(GPIO_TypeDef*)PORT_E,"PE"}, {(GPIO_TypeDef*)PORT_F,"PF"}, {(GPIO_TypeDef*)PORT_G,"PG"}, {(GPIO_TypeDef*)PORT_H,"PH"}};

// const string Pin::to_string() const {
// 	return (port_to_string.at((GPIO_TypeDef*)port) + gpio_pin_to_string.at(gpio_pin));
// }

