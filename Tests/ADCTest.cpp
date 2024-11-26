#include <gtest/gtest.h>
#include "HALALMock/Services/ADC/ADC.hpp"
#include "HALALMock/Services/SharedMemory/SharedMemory.hpp"
#include <iostream>

TEST(ADC,Inscribe){

    SharedMemory::start("GPIO_Name", "State_Machine_Name");

    //existing pin
    uint8_t adc_id_1=ADC::inscribe(PA0);
    EXPECT_EQ(adc_id_1, 0);
    //not existing pin
    uint8_t adc_id_2=ADC::inscribe(PB15);
    EXPECT_EQ(adc_id_2, 0);
    //existing pin
    uint8_t adc_id_3=ADC::inscribe(PB1);
    EXPECT_EQ(adc_id_3, 1);

    SharedMemory::close();
}

TEST(ADC,Start){
    
        SharedMemory::start("GPIO_Name", "State_Machine_Name");
    
        uint8_t adc_id_1=ADC::inscribe(PA0);
        uint8_t adc_id_2=ADC::inscribe(PB15);
    
        ADC::start();
    
        EmulatedPin& my_emulated_pin_1=SharedMemory::get_pin(PA0);
        EmulatedPin& my_emulated_pin_2=SharedMemory::get_pin(PB15);
    
        EXPECT_EQ(my_emulated_pin_1.type, PinType::ADC);
        EXPECT_EQ(my_emulated_pin_2.type, PinType::NOT_USED);
    
        SharedMemory::close();
}


TEST(ADC,Get_value){

    SharedMemory::start("GPIO_Name", "State_Machine_Name");

    uint8_t adc_id=ADC::inscribe(PA0);

    ADC::start();
    ADC::turn_on(adc_id);

    uint16_t* my_raw_value=ADC::get_value_pointer(adc_id);
    *my_raw_value=32768;

    float value=ADC::get_value(adc_id);

    EXPECT_NEAR(value, 1.65f, 0.1f);

    SharedMemory::close();
}

TEST(ADC,Get_int_value){

    SharedMemory::start("GPIO_Name", "State_Machine_Name");

    uint8_t adc_id=ADC::inscribe(PA0);

    ADC::start();
    ADC::turn_on(adc_id);

    uint16_t* my_raw_value=ADC::get_value_pointer(adc_id);
    *my_raw_value=32768;

    uint16_t raw_value=ADC::get_int_value(adc_id);

    EXPECT_EQ(raw_value, 32768);

    SharedMemory::close();
}