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

    uint8_t* PA1_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + SHM::pin_offsets[PA1]);
    uint8_t* PA0_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + SHM::pin_offsets[PA0]);
    uint8_t* PB0_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + SHM::pin_offsets[PB0]);
    uint8_t* PB3_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + SHM::pin_offsets[PB3]);
    uint8_t* PA7_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + SHM::pin_offsets[PA7]);
    uint8_t* PB2_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + SHM::pin_offsets[PB2]);

    uint8_t id_encoder = Encoder::inscribe(PA0, PA1);
    EXPECT_EQ(id_encoder, 1);
    EXPECT_EQ(Encoder::id_counter, 1);
    EXPECT_EQ(PA0_memory[0], static_cast<uint8_t>(PinType::ENCODER));
    EXPECT_EQ(PA1_memory[0], static_cast<uint8_t>(PinType::ENCODER));

    //Try to Inscribe not assigned pair pin
    id_encoder = Encoder::inscribe(PA7, PB2);
    EXPECT_EQ(id_encoder, 0);
    EXPECT_EQ(Encoder::id_counter, 1);
    EXPECT_EQ(PA7_memory[0], static_cast<uint8_t>(PinType::NOT_USED));
    EXPECT_EQ(PB2_memory[0], static_cast<uint8_t>(PinType::NOT_USED));

    //Try to Inscribe other pair pin
    id_encoder = Encoder::inscribe(PB0, PB3);
    EXPECT_EQ(id_encoder, 2);
    EXPECT_EQ(Encoder::id_counter, 2);
    EXPECT_EQ(PB0_memory[0], static_cast<uint8_t>(PinType::ENCODER));
    EXPECT_EQ(PB3_memory[0], static_cast<uint8_t>(PinType::ENCODER));

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
    uint8_t* pin_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + SHM::pin_offsets[PA0]);

    ASSERT_FALSE(*reinterpret_cast<bool*>(pin_memory+6));
    Encoder::turn_on(id_encoder);
    ASSERT_TRUE(*reinterpret_cast<bool*>(pin_memory+6));

    SharedMemory::close();
}

TEST(EncoderTest, TurnOff)
{
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    uint8_t id_encoder = Encoder::inscribe(PA0, PA1);
    uint8_t* pin_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + SHM::pin_offsets[PA0]);
    
    ASSERT_FALSE(*reinterpret_cast<bool*>(pin_memory+6));
    Encoder::turn_on(id_encoder);
    
    ASSERT_TRUE(*reinterpret_cast<bool*>(pin_memory+6));
    
    Encoder::turn_off(id_encoder);
    ASSERT_FALSE(*reinterpret_cast<bool*>(pin_memory+6));
    
    SharedMemory::close();
}

TEST(EncoderTest, reset)
{
    SharedMemory::start("GPIO_Name", "State_Machine_Name");

    uint8_t id_encoder = Encoder::inscribe(PA0, PA1);
    uint8_t* pin_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + SHM::pin_offsets[PA0]);

    Encoder::reset(id_encoder);
    EXPECT_EQ(*reinterpret_cast<uint32_t*>(pin_memory+1), UINT32_MAX / 2);

    SharedMemory::close();

}

TEST(EncoderTest, getDireccion)
{
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    uint8_t id_encoder = Encoder::inscribe(PA0, PA1);
    uint8_t* pin_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + SHM::pin_offsets[PA0]);

    //write true to check it
    *reinterpret_cast<bool*>(pin_memory+5) = true;
    bool direction = Encoder::get_direction(id_encoder);
    ASSERT_TRUE(direction);

    *reinterpret_cast<bool*>(pin_memory+5) = false;
    direction = Encoder::get_direction(id_encoder);
    ASSERT_FALSE(direction);

    SharedMemory::close();
    
}

TEST(EncoderTest, getCounter)
{
    SharedMemory::start("GPIO_Name", "State_Machine_Name");

    uint8_t id_encoder = Encoder::inscribe(PA0, PA1);
    uint8_t* pin_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + SHM::pin_offsets[PA0]);

    uint32_t value = 1000;
    *reinterpret_cast<uint32_t*>(pin_memory+1) = value;

    uint32_t counter_value = Encoder::get_counter(id_encoder);

    EXPECT_EQ(value, counter_value);

    SharedMemory::close();
    
}
