/*
 * EXTI.cpp
 *
 *  Created on: Nov 5, 2022
 *      Author: alejandro
 */

#include "HALAL/Services/EXTI/EXTI.hpp"

#include "ErrorHandler/ErrorHandler.hpp"

ST_LIB::EXTIDomain::Instance* ST_LIB::EXTIDomain::g_instances[ST_LIB::EXTIDomain::max_instances] = {
    nullptr};

extern "C"{

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    auto index = ST_LIB::EXTIDomain::get_pin_index(GPIO_Pin);
    if (index >= ST_LIB::EXTIDomain::max_instances) return;
    auto* exti = ST_LIB::EXTIDomain::g_instances[index];
    if (exti && exti->is_on && exti->action) {
        exti->action();
    }
}

void EXTI0_IRQHandler() {
    HAL_GPIO_EXTI_IRQHandler(0x0001);
}

void EXTI1_IRQHandler() { HAL_GPIO_EXTI_IRQHandler(0x0002); }
void EXTI2_IRQHandler() { HAL_GPIO_EXTI_IRQHandler(0x0004); }
void EXTI3_IRQHandler() { HAL_GPIO_EXTI_IRQHandler(0x0008); }
void EXTI4_IRQHandler() { HAL_GPIO_EXTI_IRQHandler(0x0010); }
void EXTI9_5_IRQHandler() { 
    for (uint16_t pin_mask = 0x0020; pin_mask <= 0x0200; pin_mask <<= 1) {
        if (__HAL_GPIO_EXTI_GET_IT(pin_mask) != 0x00U) { HAL_GPIO_EXTI_IRQHandler(pin_mask); }
    }
}
void EXTI15_10_IRQHandler() {
    for (uint16_t pin_mask = 0x0400; pin_mask <= 0x8000; pin_mask <<= 1) {
        if (__HAL_GPIO_EXTI_GET_IT(pin_mask) != 0x00U) { HAL_GPIO_EXTI_IRQHandler(pin_mask); }
    }
}

}
