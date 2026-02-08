#pragma once

#include <cstdint>

#include "HALAL/Services/ADC/NewADC.hpp"

namespace ST_LIB {
inline uint8_t adc_resolution_bits(ADCDomain::Resolution r) {
  switch (r) {
  case ADCDomain::Resolution::BITS_16:
    return 16;
  case ADCDomain::Resolution::BITS_14:
    return 14;
  case ADCDomain::Resolution::BITS_12:
    return 12;
  case ADCDomain::Resolution::BITS_10:
    return 10;
  case ADCDomain::Resolution::BITS_8:
    return 8;
  }
  return 12;
}

inline uint32_t adc_max_value(ADCDomain::Resolution r) {
  const uint8_t bits = adc_resolution_bits(r);
  if (bits >= 32) {
    return 0xFFFFFFFFu;
  }
  return (static_cast<uint32_t>(1u) << bits) - 1u;
}

inline float adc_raw_to_voltage(uint32_t raw, ADCDomain::Resolution r,
                                float vref = 3.3f) {
  const uint32_t max_val = adc_max_value(r);
  if (max_val == 0) {
    return 0.0f;
  }
  return (static_cast<float>(raw) / static_cast<float>(max_val)) * vref;
}

inline uint16_t adc_raw_to_12bit(uint32_t raw, ADCDomain::Resolution r) {
  const uint8_t bits = adc_resolution_bits(r);
  if (bits == 12) {
    return static_cast<uint16_t>(raw & 0x0FFFu);
  }
  if (bits > 12) {
    return static_cast<uint16_t>(raw >> (bits - 12));
  }
  return static_cast<uint16_t>((raw << (12 - bits)) & 0x0FFFu);
}
} // namespace ST_LIB
