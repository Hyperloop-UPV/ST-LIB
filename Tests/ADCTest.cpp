#include <gtest/gtest.h>
#include "HALALMock/Services/ADC/ADC.hpp"
#include "HALALMock/Services/SharedMemory/SharedMemory.hpp"
#include <iostream>

TEST(ADC, Check_Memory_Writing){

    SharedMemory::start("GPIO_Name", "State_Machine_Name");

    for (int i = 0; i < SharedMemory::total_pins; i++) {
        ((uint8_t *)(SharedMemory::gpio_memory + i))[1]=1;
        EXPECT_EQ(((uint8_t *)(SharedMemory::gpio_memory + i))[1],1);
        ((uint8_t *)(SharedMemory::gpio_memory + i))[1]=0;
        EXPECT_EQ(((uint8_t *)(SharedMemory::gpio_memory + i))[1],0);
    }

    SharedMemory::close();

}

TEST(ADC,Inscribe){

    SharedMemory::start("GPIO_Name", "State_Machine_Name");

    //existing pin
    uint8_t adc_id_1=ADC::inscribe(PA0);
    EXPECT_EQ(adc_id_1, 0);
    uint8_t* PA0_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + SHM::pin_offsets[PA0]);
    EXPECT_EQ(PA0_memory[0], static_cast<uint8_t>(PinType::ADC));

    //not existing pin
    uint8_t adc_id_2=ADC::inscribe(PB15);
    EXPECT_EQ(adc_id_2, 0);
    uint8_t* PB15_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + SHM::pin_offsets[PB15]);
    EXPECT_EQ(PB15_memory[0], static_cast<uint8_t>(PinType::NOT_USED));

    //existing pin
    uint8_t adc_id_3=ADC::inscribe(PB1);
    EXPECT_EQ(adc_id_3, 1);
    uint8_t* PB1_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + SHM::pin_offsets[PB1]);
    EXPECT_EQ(PB1_memory[0], static_cast<uint8_t>(PinType::ADC));

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

TEST(ADC,Get_value_Memory){

    SharedMemory::start("GPIO_Name", "State_Machine_Name");

    uint8_t adc_id=ADC::inscribe(PA0);

    ADC::start();
    ADC::turn_on(adc_id);

    

    uint8_t offset=SHM::pin_offsets[PA0];
    uint8_t* pin_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + offset);

    // check if turn_on() is setting the bool_is_on to true
    bool* bool_is_on=reinterpret_cast<bool*>(pin_memory+3);
    EXPECT_EQ(*bool_is_on,true);

    uint16_t* adc_value=reinterpret_cast<uint16_t*>(pin_memory+1);
    *adc_value=32768;

    float value=ADC::get_value(adc_id);

    EXPECT_NEAR(value, 1.65f, 0.1f);

    SharedMemory::close();
}

TEST(ADC,Get_int_value){

    SharedMemory::start("GPIO_Name", "State_Machine_Name");

    uint8_t adc_id=ADC::inscribe(PB1);

    ADC::start();
    ADC::turn_on(adc_id);

    uint16_t* my_raw_value=ADC::get_value_pointer(adc_id);
    *my_raw_value=32768;

    uint16_t raw_value=ADC::get_int_value(adc_id);

    EXPECT_EQ(raw_value, 32768);

    SharedMemory::close();
}

TEST(ADC,Get_int_value_Memory){

    SharedMemory::start("GPIO_Name", "State_Machine_Name");

    uint8_t adc_id=ADC::inscribe(PB1);

    ADC::start();
    ADC::turn_on(adc_id);

    uint8_t offset=SHM::pin_offsets[PB1];
    uint8_t* pin_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + offset);

    // check if turn_on() is setting the bool_is_on to true
    bool* bool_is_on=reinterpret_cast<bool*>(pin_memory+3);
    EXPECT_EQ(*bool_is_on,true);

    uint16_t* adc_value=reinterpret_cast<uint16_t*>(pin_memory+1);
    *adc_value=32768;

    uint16_t raw_value=ADC::get_int_value(adc_id);

    EXPECT_EQ(raw_value, 32768);

    SharedMemory::close();
}