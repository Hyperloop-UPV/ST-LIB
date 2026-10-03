#include "HALAL/Models/TimerDomain/TimerDomain.hpp"

using namespace ST_LIB;

#define CaptureCompareInterruptMask (TIM_SR_CC1IF | TIM_SR_CC2IF | TIM_SR_CC3IF | TIM_SR_CC4IF)

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim5;
TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim7;
TIM_HandleTypeDef htim8;
TIM_HandleTypeDef htim12;
TIM_HandleTypeDef htim13;
TIM_HandleTypeDef htim14;
TIM_HandleTypeDef htim15;
TIM_HandleTypeDef htim16;
TIM_HandleTypeDef htim17;
TIM_HandleTypeDef htim23;
TIM_HandleTypeDef htim24;

void (*TimerDomain::callbacks[TimerDomain::max_instances])(void*) = {nullptr};
void* TimerDomain::callback_data[TimerDomain::max_instances] = {nullptr};

ST_LIB::InputCaptureInfo* TimerDomain::input_capture_info[max_instances][input_capture_channels];
ST_LIB::InputCaptureInfo TimerDomain::input_capture_info_backing[max_instances]
                                                                [input_capture_channels];
ST_LIB::InputCaptureInfo TimerDomain::input_capture_info_dummy;

static void TIM_IC_CaptureCallback(const uint32_t timer_idx, uint32_t channel) {
    TIM_HandleTypeDef* htim = TimerDomain::hal_handles[timer_idx];

    ST_LIB::InputCaptureInfo* info = TimerDomain::input_capture_info[timer_idx][channel];
    uint32_t current = (*(((volatile uint32_t*)&htim->Instance->CCR1) + channel));
    if (info->channel_rising == channel) {
        // NOTE: CCR1 - CCR4 are contiguous
        // NOTE: CCxIF flag is cleared by software by reading the captured data in CCRx
        uint32_t period;
        if (current >= info->value_rising) {
            period = current - info->value_rising;
        } else {
            // counter wrap around
            uint32_t max_count = htim->Instance->ARR - 1;
            period = (current + max_count) - info->value_rising;
        }

        if ((period != 0) && (info->value_falling < period)) {
            uint32_t ref_clock =
                TimerDomain::get_timer_frequency(htim->Instance) / (htim->Instance->PSC + 1);
            info->period = period;
            info->frequency = ref_clock / period;
            info->duty_cycle = ((float)info->value_falling * 100.0f) / (float)period;
        }
        info->value_rising = current;
    } else if (info->channel_falling == channel) {
        uint32_t falling_value;
        if (current >= info->value_rising) {
            falling_value = current - info->value_rising;
        } else {
            uint32_t max_count = htim->Instance->ARR - 1;
            falling_value = (current + max_count) - info->value_rising;
        }

        if (falling_value < info->period)
            info->value_falling = falling_value;
    } else [[unlikely]] {
        // TimerDomain::input_capture_info was modified or STM interrupts are all over the place
        // either way, this is a no-op
        // ErrorHandler("TimerDomain::input_capture_info was modified");
    }
}

static void TIM_InterruptCallback(const uint32_t timer_idx) {
    TIM_TypeDef* tim = TimerDomain::cmsis_timers[timer_idx];
    if (tim->SR & TIM_SR_UIF) {
        CLEAR_BIT(tim->SR, TIM_SR_UIF);
        TimerDomain::callbacks[timer_idx](TimerDomain::callback_data[timer_idx]);
    }

    // NOTE: possible optimization: only do the channels possible for timer
    // Bit 0 = UIF, bits 1 - 4 = CCxIF
    for (uint32_t ch = 1; ch < 5; ch++) {
        uint32_t flag_mask = 1U << ch;
        if (tim->SR & flag_mask) {
            TIM_IC_CaptureCallback(timer_idx, ch - 1);
            CLEAR_BIT(tim->SR, flag_mask);
        }
    }
}

extern "C" void TIM1_UP_IRQHandler(void) { TIM_InterruptCallback(timer_idxmap[1]); }

extern "C" void TIM2_IRQHandler(void) { TIM_InterruptCallback(timer_idxmap[2]); }

extern "C" void TIM3_IRQHandler(void) { TIM_InterruptCallback(timer_idxmap[3]); }

extern "C" void TIM4_IRQHandler(void) { TIM_InterruptCallback(timer_idxmap[4]); }

extern "C" void TIM5_IRQHandler(void) { TIM_InterruptCallback(timer_idxmap[5]); }

extern "C" void TIM6_DAC_IRQHandler(void) {
    // NOTE: Basic timers have no capture compare channels
    CLEAR_BIT(TimerDomain::cmsis_timers[timer_idxmap[6]]->SR, TIM_SR_UIF);
    TimerDomain::callbacks[timer_idxmap[6]](TimerDomain::callback_data[timer_idxmap[6]]);
}

extern "C" void TIM7_IRQHandler(void) {
    // NOTE: Basic timers have no capture compare channels
    CLEAR_BIT(TimerDomain::cmsis_timers[timer_idxmap[7]]->SR, TIM_SR_UIF);
    TimerDomain::callbacks[timer_idxmap[7]](TimerDomain::callback_data[timer_idxmap[7]]);
}

extern "C" void TIM8_BRK_TIM12_IRQHandler(void) {
    constexpr uint32_t tim8_idx = timer_idxmap[8];
    constexpr uint32_t tim12_idx = timer_idxmap[12];

    TIM_TypeDef* tim8 = TimerDomain::cmsis_timers[tim8_idx];
    TIM_TypeDef* tim12 = TimerDomain::cmsis_timers[tim12_idx];

    if ((tim12->SR & TIM_SR_UIF) != 0) {
        CLEAR_BIT(tim12->SR, TIM_SR_UIF);
        TimerDomain::callbacks[tim12_idx](TimerDomain::callback_data[tim12_idx]);
    }
    // Bit 0 = UIF, bits 1 - 4 = CCxIF
    for (uint32_t ch = 1; ch < 5; ch++) {
        uint32_t flag_mask = 1U << ch;
        if (tim12->SR & flag_mask) {
            TIM_IC_CaptureCallback(tim12_idx, ch - 1);
            CLEAR_BIT(tim12->SR, flag_mask);
        }
    }

    if ((tim8->SR & TIM_SR_BIF) != 0) {
        CLEAR_BIT(tim8->SR, TIM_SR_BIF);
        /* this could probably have some other callback */
        TimerDomain::callbacks[tim8_idx](TimerDomain::callback_data[tim8_idx]);
    }
}

extern "C" void TIM8_UP_TIM13_IRQHandler(void) {
    constexpr uint32_t tim8_idx = timer_idxmap[8];
    constexpr uint32_t tim13_idx = timer_idxmap[13];

    TIM_TypeDef* tim8 = TimerDomain::cmsis_timers[tim8_idx];
    TIM_TypeDef* tim13 = TimerDomain::cmsis_timers[tim13_idx];

    if ((tim13->SR & TIM_SR_UIF) != 0) {
        CLEAR_BIT(tim13->SR, TIM_SR_UIF);
        TimerDomain::callbacks[tim13_idx](TimerDomain::callback_data[tim13_idx]);
    }
    // Bit 0 = UIF, bits 1 - 4 = CCxIF
    for (uint32_t ch = 1; ch < 5; ch++) {
        uint32_t flag_mask = 1U << ch;
        if (tim13->SR & flag_mask) {
            TIM_IC_CaptureCallback(tim13_idx, ch - 1);
            CLEAR_BIT(tim13->SR, flag_mask);
        }
    }

    if ((tim8->SR & TIM_SR_UIF) != 0) {
        CLEAR_BIT(tim8->SR, TIM_SR_UIF);
        TimerDomain::callbacks[tim8_idx](TimerDomain::callback_data[tim8_idx]);
    }
    // Bit 0 = UIF, bits 1 - 4 = CCxIF
    for (uint32_t ch = 1; ch < 5; ch++) {
        uint32_t flag_mask = 1U << ch;
        if (tim8->SR & flag_mask) {
            TIM_IC_CaptureCallback(tim8_idx, ch - 1);
            CLEAR_BIT(tim8->SR, flag_mask);
        }
    }
}

extern "C" void TIM8_TRG_COM_TIM14_IRQHandler(void) {
    constexpr uint32_t tim8_idx = timer_idxmap[8];
    constexpr uint32_t tim14_idx = timer_idxmap[14];

    TIM_TypeDef* tim8 = TimerDomain::cmsis_timers[tim8_idx];
    TIM_TypeDef* tim14 = TimerDomain::cmsis_timers[tim14_idx];

    if ((tim14->SR & TIM_SR_UIF) != 0) {
        CLEAR_BIT(tim14->SR, TIM_SR_UIF);
        TimerDomain::callbacks[tim14_idx](TimerDomain::callback_data[tim14_idx]);
    }
    // Bit 0 = UIF, bits 1 - 4 = CCxIF
    for (uint32_t ch = 1; ch < 5; ch++) {
        uint32_t flag_mask = 1U << ch;
        if (tim14->SR & flag_mask) {
            TIM_IC_CaptureCallback(tim14_idx, ch - 1);
            CLEAR_BIT(tim14->SR, flag_mask);
        }
    }

    constexpr uint32_t com_trg_flags = TIM_SR_TIF | TIM_SR_COMIF;
    if ((tim8->SR & com_trg_flags) != 0) {
        CLEAR_BIT(tim8->SR, com_trg_flags);
        /* this could probably have some other callback */
        TimerDomain::callbacks[tim8_idx](TimerDomain::callback_data[tim8_idx]);
    }
}

extern "C" void TIM15_IRQHandler(void) { TIM_InterruptCallback(timer_idxmap[15]); }

extern "C" void TIM16_IRQHandler(void) { TIM_InterruptCallback(timer_idxmap[16]); }

extern "C" void TIM17_IRQHandler(void) { TIM_InterruptCallback(timer_idxmap[17]); }

extern "C" void TIM23_IRQHandler(void) { TIM_InterruptCallback(timer_idxmap[23]); }

extern "C" void TIM24_IRQHandler(void) { TIM_InterruptCallback(timer_idxmap[24]); }
