#include <gtest/gtest.h>
#include "HALALMock/Services/Encoder/Encoder.hpp"
#include "HALALMock/Services/SharedMemory/SharedMemory.hpp"

map<pair<Pin, Pin>, void*> Encoder::pin_timer_map = {
    {{PA0, PA1}, nullptr}, {{PB0, PB3}, nullptr}
};



TEST(EncoderTest, Inscribe)
{
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    
    //Try to Inscribe normal pair pin
    uint8_t id_encoder = Encoder::inscribe(PA0, PA1);
    EXPECT_EQ(id_encoder, 1);
    EXPECT_EQ(Encoder::id_counter, 1);

    //Try to Inscribe not assigned pair pin
    id_encoder = Encoder::inscribe(PA7, PB2);
    EXPECT_EQ(id_encoder, 0);
    EXPECT_EQ(Encoder::id_counter, 1);

    //Try to Inscribe other pair pin
    id_encoder = Encoder::inscribe(PB0, PB3);
    EXPECT_EQ(id_encoder, 2);
    EXPECT_EQ(Encoder::id_counter, 2);

    //Try to Inscribe a repeteded pair pin
    id_encoder = Encoder::inscribe(PB0, PB3);
    EXPECT_EQ(id_encoder, 0);
    EXPECT_EQ(Encoder::id_counter, 2);

    SharedMemory::close();

}

TEST(EncoderTest, TurnOn)
{
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    uint8_t id_encoder = Encoder::inscribe(PA0, PA1);
    EmulatedPin &pin = SharedMemory::get_pin(PA0);

    ASSERT_FALSE(pin.PinData.encoder.is_on);
    Encoder::turn_on(id_encoder);
    ASSERT_TRUE(pin.PinData.encoder.is_on);

    SharedMemory::close();
}

TEST(EncoderTest, TurnOff)
{
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    uint8_t id_encoder = Encoder::inscribe(PA0, PA1);
    EmulatedPin &pin = SharedMemory::get_pin(PA0);
    
    ASSERT_FALSE(pin.PinData.encoder.is_on);
    Encoder::turn_on(id_encoder);
    
    ASSERT_TRUE(pin.PinData.encoder.is_on);
    
    Encoder::turn_off(id_encoder);
    ASSERT_FALSE(pin.PinData.encoder.is_on);
    
    SharedMemory::close();
}

TEST(EncoderTest, reset)
{
    SharedMemory::start("GPIO_Name", "State_Machine_Name");

    uint8_t id_encoder = Encoder::inscribe(PA0, PA1);
    EmulatedPin &pin = SharedMemory::get_pin(PA0);

    Encoder::reset(id_encoder);
    EXPECT_EQ(pin.PinData.encoder.count_value, UINT32_MAX / 2);

    SharedMemory::close();

}

TEST(EncoderTest, getDireccion)
{
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    uint8_t id_encoder = Encoder::inscribe(PA0, PA1);
    EmulatedPin &pin = SharedMemory::get_pin(PA0);

    //write true to check it
    pin.PinData.encoder.direction = true;
    bool direction = Encoder::get_direction(id_encoder);
    ASSERT_TRUE(direction);

    pin.PinData.encoder.direction = false;
    direction = Encoder::get_direction(id_encoder);
    ASSERT_FALSE(direction);

    SharedMemory::close();
    
}

TEST(EncoderTest, getCounter)
{
    SharedMemory::start("GPIO_Name", "State_Machine_Name");

    uint8_t id_encoder = Encoder::inscribe(PA0, PA1);
    EmulatedPin &pin = SharedMemory::get_pin(PA0);

    uint32_t value = 1000;
    pin.PinData.encoder.count_value = value;

    uint32_t counter_value = Encoder::get_counter(id_encoder);

    EXPECT_EQ(value, counter_value);

    SharedMemory::close();
    
}
