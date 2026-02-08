#include "Sensors/NTC/NTC.hpp"

NTC::NTC(ST_LIB::ADCDomain::Instance& adc, float *src)
    : value(src), adc(&adc) {}
NTC::NTC(ST_LIB::ADCDomain::Instance& adc, float& src)
    : value(&src), adc(&adc) {}

void NTC::read() {
    if (adc == nullptr || value == nullptr) {
        return;
    }
    const uint32_t raw = adc->read_raw();
    const uint16_t val = ST_LIB::adc_raw_to_12bit(raw, adc->resolution);
    *value = static_cast<float>(NTC_table[val]) * 0.1f;
}
