/*
 * DualPWM.cpp
 *
 *  Created on: Feb 27, 2023
 *      Author: aleja
 */

#include "HALAL/Services/PWM/DualPWM/DualPWM.hpp"
#include "ErrorHandler/ErrorHandler.hpp"
#include "stm32h7xx_hal_def.h"

DualPWM::DualPWM(Pin& pin, Pin& pin_negated) {
    if (not TimerPeripheral::available_dual_pwms.contains({pin, pin_negated})) {
        ErrorHandler(
            "Pins %s and %s are not registered as an available Dual PWM",
            pin.to_string(),
            pin_negated.to_string()
        );
    }

    TimerPeripheral& timer = TimerPeripheral::available_dual_pwms.at({pin, pin_negated}).first;
    TimerPeripheral::PWMData& pwm_data =
        TimerPeripheral::available_dual_pwms.at({pin, pin_negated}).second;

    peripheral = &timer;
    channel = pwm_data.channel;

    if (pwm_data.mode != TimerPeripheral::PWM_MODE::NORMAL) {
        ErrorHandler(
            "Pins %s and %s are not registered as a DUAL PWM",
            pin.to_string(),
            pin_negated.to_string()
        );
    }

    Pin::inscribe(pin, TIMER_ALTERNATE_FUNCTION);
    Pin::inscribe(pin_negated, TIMER_ALTERNATE_FUNCTION);
    timer.init_data.pwm_channels.push_back(pwm_data);

    duty_cycle = 0;
}

void DualPWM::turn_on() {
    if (HAL_TIM_PWM_Start(peripheral->handle, channel) != HAL_OK) {
        ErrorHandler("Dual PWM positive channel did not start correctly", 0);
    }

    if (HAL_TIMEx_PWMN_Start(peripheral->handle, channel) != HAL_OK) {
        ErrorHandler("Dual PWM negative channel did not start correctly", 0);
    }
}

void DualPWM::turn_off() {

    if (HAL_TIM_PWM_Stop(peripheral->handle, channel) != HAL_OK) {
        ErrorHandler("Dual PWM positive channel did not stop correctly", 0);
    }

    if (HAL_TIMEx_PWMN_Stop(peripheral->handle, channel) != HAL_OK) {
        ErrorHandler("Dual PWM negative channel did not stop correctly", 0);
    }
}

void DualPWM::turn_on_positive() {
    if (HAL_TIM_PWM_Start(peripheral->handle, channel) != HAL_OK) {
        ErrorHandler("Dual PWM positive channel did not start correctly", 0);
    }
}

void DualPWM::turn_on_negated() {
    if (HAL_TIMEx_PWMN_Start(peripheral->handle, channel) != HAL_OK) {
        ErrorHandler("Dual PWM negative channel did not start correctly", 0);
    }
}

void DualPWM::turn_off_positive() {
    if (HAL_TIM_PWM_Stop(peripheral->handle, channel) != HAL_OK) {
        ErrorHandler("Dual PWM positive channel did not stop correctly", 0);
    }
}

void DualPWM::turn_off_negated() {
    if (HAL_TIMEx_PWMN_Stop(peripheral->handle, channel) != HAL_OK) {
        ErrorHandler("Dual PWM negative channel did not stop correctly", 0);
    }
}

void DualPWM::set_duty_cycle(float duty_cycle) {
    uint16_t raw_duty = __HAL_TIM_GET_AUTORELOAD(peripheral->handle) / 100.0 * duty_cycle;
    __HAL_TIM_SET_COMPARE(peripheral->handle, channel, raw_duty);
    this->duty_cycle = duty_cycle;
}
void DualPWM::set_frequency(uint32_t freq_in_hz) {
    this->frequency = freq_in_hz;
    TIM_TypeDef& timer = *peripheral->handle->Instance;
    timer.ARR = (HAL_RCC_GetPCLK1Freq() * 2 / (timer.PSC + 1)) / frequency;
    set_duty_cycle(duty_cycle);
}
uint32_t DualPWM::get_frequency() const { return frequency; }
float DualPWM::get_duty_cycle() const { return duty_cycle; }
void DualPWM::set_dead_time(std::chrono::nanoseconds dead_time_ns) {
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

    // per https://hasanyavuz.ozderya.net/?p=437
    auto time = dead_time_ns.count();

    if (time <= 127 * clock_period_ns) {
        sBreakDeadTimeConfig.DeadTime = time / clock_period_ns;
    } else if (time > 127 * clock_period_ns && time <= 2 * clock_period_ns * 127) {
        sBreakDeadTimeConfig.DeadTime = time / (2 * clock_period_ns) - 64 + 128;
    } else if (time > 2 * clock_period_ns * 127 && time <= 8 * clock_period_ns * 127) {
        sBreakDeadTimeConfig.DeadTime = time / (8 * clock_period_ns) - 32 + 192;
    } else if (time > 8 * clock_period_ns * 127 && time <= 16 * clock_period_ns * 127) {
        sBreakDeadTimeConfig.DeadTime = time / (16 * clock_period_ns) - 32 + 224;
    } else {
        ErrorHandler("Invalid dead time configuration");
    }
    // sBreakDeadTimeConfig.LockLevel = 0;
    // sBreakDeadTimeConfig.BreakState = 1;
    HAL_TIMEx_ConfigBreakDeadTime(peripheral->handle, &sBreakDeadTimeConfig);
    peripheral->handle->Instance->BDTR |= TIM_BDTR_MOE;
    return;
}
