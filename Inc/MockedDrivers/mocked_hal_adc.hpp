#pragma once

#include "hal_wrapper.h"

#include <cstdint>

namespace ST_LIB::MockedHAL {

void adc_reset();

void adc_set_channel_raw(ADC_TypeDef* adc, uint32_t channel, uint32_t raw_value);

void adc_set_poll_timeout(ADC_TypeDef* adc, bool enabled);

uint32_t adc_get_last_channel(ADC_TypeDef* adc);

bool adc_is_running(ADC_TypeDef* adc);

} // namespace ST_LIB::MockedHAL
