/*
 * Pin.cpp
 *
 *  Created on: 19 oct. 2022
 *      Author: stefan
 */

#include "HALAL/Models/PinModel/Pin.hpp"

#include "ErrorHandler/ErrorHandler.hpp"


// const map<GPIOPin, const string> Pin::gpio_pin_to_string = {
//     {PIN_0, "0"},    {PIN_1, "1"},   {PIN_2, "2"},   {PIN_3, "3"},
//     {PIN_4, "4"},    {PIN_5, "5"},   {PIN_6, "6"},   {PIN_7, "7"},
//     {PIN_8, "8"},    {PIN_9, "9"},   {PIN_10, "10"}, {PIN_11, "11"},
//     {PIN_12, "12"},  {PIN_13, "13"}, {PIN_14, "14"}, {PIN_15, "15"},
//     {PIN_ALL, "ALL"}};
// const map<GPIO_TypeDef*, const string> Pin::port_to_string = {
//     {(GPIO_TypeDef*)PORT_A, "PA"}, {(GPIO_TypeDef*)PORT_B, "PB"},
//     {(GPIO_TypeDef*)PORT_C, "PC"}, {(GPIO_TypeDef*)PORT_D, "PD"},
//     {(GPIO_TypeDef*)PORT_E, "PE"}, {(GPIO_TypeDef*)PORT_F, "PF"},
//     {(GPIO_TypeDef*)PORT_G, "PG"}, {(GPIO_TypeDef*)PORT_H, "PH"}};

// const string Pin::to_string() const {
//     return (port_to_string.at(port) + gpio_pin_to_string.at(gpio_pin));
// }

void Pin::start() {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();

    for (auto pin : pinArray) {
        HAL_GPIO_Init(getGPIO(pin->port), &pin->GPIO_InitStruct);
    }
}

