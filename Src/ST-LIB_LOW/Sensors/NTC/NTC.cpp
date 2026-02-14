#include "Sensors/NTC/NTC.hpp"

NTC::NTC(ST_LIB::ADCDomain::Instance& adc, float* src) : value(src), adc(&adc) {}
NTC::NTC(ST_LIB::ADCDomain::Instance& adc, float& src) : value(&src), adc(&adc) {}

void NTC::read() {
    if (adc == nullptr || value == nullptr) {
        return;
    }
    const float raw = adc->get_raw();
    uint16_t val = static_cast<uint16_t>(adc->get_value_from_raw(raw, 4095.0f));
    if (val > 4095u) {
        val = 4095u;
    }
    *value = static_cast<float>(NTC_table[val]) * 0.1f;
}
