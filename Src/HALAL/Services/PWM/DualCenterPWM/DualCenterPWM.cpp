#include "HALAL/Services/PWM/DualCenterPWM/DualCenterPWM.hpp"

DualCenterPWM::DualCenterPWM(Pin& pin, Pin& pin_negated) {
    if (not TimerPeripheral::available_dual_pwms.contains({pin, pin_negated})) {
        ErrorHandler(
            "Pins %s and %s are not registered as an available Dual PWM",
            pin.to_string(), pin_negated.to_string());
    }

    TimerPeripheral& timer =
        TimerPeripheral::available_dual_pwms.at({pin, pin_negated}).first;
    TimerPeripheral::PWMData& pwm_data =
        TimerPeripheral::available_dual_pwms.at({pin, pin_negated}).second;

    peripheral = &timer;
    channel = pwm_data.channel;

    if (pwm_data.mode != TimerPeripheral::PWM_MODE::CENTER_ALIGNED) {
        ErrorHandler(
            "Pins %s and %s are not registered as a CENTER ALIGNED PWM",
            pin.to_string(), pin_negated.to_string());
    }

    Pin::inscribe(pin, TIMER_ALTERNATE_FUNCTION);
    Pin::inscribe(pin_negated, TIMER_ALTERNATE_FUNCTION);
    timer.init_data.pwm_channels.push_back(pwm_data);

    duty_cycle = 0;
}

void DualCenterPWM::set_frequency(uint32_t freq_in_hz) {
    this->frequency = freq_in_hz;
    TIM_TypeDef& timer = *peripheral->handle->Instance;
    timer.ARR =
        (HAL_RCC_GetPCLK1Freq() * 2 / (timer.PSC + 1)) / (frequency * 2);
    set_duty_cycle(duty_cycle);
}