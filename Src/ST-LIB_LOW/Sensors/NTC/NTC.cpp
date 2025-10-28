#include "Sensors/NTC/NTC.hpp"
#ifdef NTC_
NTC::NTC(Pin &pin,float *src):value(src){
    id = ADC::inscribe(pin);

    Sensor::adc_id_list.push_back(id);
}
NTC::NTC(Pin &pin,float& src):value(&src){
    id = ADC::inscribe(pin);

    Sensor::adc_id_list.push_back(id);
}

uint8_t NTC::get_id(){
    return id;
}

void NTC::read() {
    int val = ADC::get_int_value(id) >> 4;

    *value =  NTC_table[val] * 0.1;

    
}
#endif