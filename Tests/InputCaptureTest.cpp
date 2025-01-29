#include <gtest/gtest.h>
#include "HALALMock/Services/InputCapture/InputCapture.hpp"
#include "HALALMock/Services/SharedMemory/SharedMemory.hpp"

#define TIM_CHANNEL_1 1
#define TIM_CHANNEL_2 2
#define htim nullptr

map<Pin, InputCapture::Instance> InputCapture::available_instances = {
    {PF0, InputCapture::Instance(PF0, htim, TIM_CHANNEL_1, TIM_CHANNEL_2)},
    {PB0, InputCapture::Instance(PB0, htim, TIM_CHANNEL_1, TIM_CHANNEL_2)},
    {PE1, InputCapture::Instance(PE1, htim, TIM_CHANNEL_1, TIM_CHANNEL_2)},
    {PE2, InputCapture::Instance(PE2, htim, TIM_CHANNEL_1, TIM_CHANNEL_2)}
    };

static uint8_t id1, id2;
TEST(InputCapture, Inscribe){

    SharedMemory::start("GPIO_Name", "State_Machine_Name");

    //inscribe correct PIN
    id1 = InputCapture::inscribe(PF0);

    uint8_t* PF0_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + SHM::pin_offsets[PF0]);
    uint8_t* PA0_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + SHM::pin_offsets[PA0]);
    uint8_t* PB0_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + SHM::pin_offsets[PB0]);

    EXPECT_EQ(PF0_memory[0], static_cast<uint8_t>(PinType::INPUTCAPTURE));
    EXPECT_EQ(id1, 1);
    EXPECT_EQ(InputCapture::id_counter, 1);

    //inscribe wrong PIN
    uint8_t id = InputCapture::inscribe(PA0);
    EXPECT_EQ(PA0_memory[0], static_cast<uint8_t>(PinType::NOT_USED));
    EXPECT_EQ(id, 0);
    EXPECT_EQ(InputCapture::id_counter, 1);

    //inscribe another correct PIN
    id2 = InputCapture::inscribe(PB0);
    EXPECT_EQ(PB0_memory[0], static_cast<uint8_t>(PinType::INPUTCAPTURE));
    EXPECT_EQ(id2, 2);
    EXPECT_EQ(InputCapture::id_counter, 2);

    //inscribe repeated PIN
    id = InputCapture::inscribe(PF0);
    EXPECT_EQ(id, 0);
    EXPECT_EQ(InputCapture::id_counter, 2);

}

TEST(InputCapture, TurnOn){


    EmulatedPin &pin = SharedMemory::get_pin(PF0);

    bool is_on = false;
    is_on = *(InputCapture::active_instances[id1].is_on);
    ASSERT_FALSE(is_on);
    InputCapture::turn_on(id1);
    is_on = *(InputCapture::active_instances[id1].is_on);
    ASSERT_TRUE(is_on);

}

TEST(InputCapture, TurnOff){

    EmulatedPin &pin = SharedMemory::get_pin(PF0);

    InputCapture::turn_on(id1);

    bool is_on;
    is_on = *(InputCapture::active_instances[id1].is_on);
    InputCapture::turn_on(id1);
    ASSERT_TRUE(is_on);

    InputCapture::turn_off(id1);
    is_on = *(InputCapture::active_instances[id1].is_on);
    ASSERT_FALSE(is_on);

}

TEST(InputCapture, ReadFrequency){

    uint8_t* PE1_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + SHM::pin_offsets[PE1]);
    uint8_t id = InputCapture::inscribe(PE1);

    uint32_t value = 1000;
    *reinterpret_cast<uint32_t*>(PE1_memory+2) = value;

    //case with correct PIN and active Instance
    InputCapture::turn_on(id);
    uint32_t frequency = InputCapture::read_frequency(id);
    EXPECT_EQ(value, frequency);

    //case with correct PIN and deactive Instance
    InputCapture::turn_off(id);
    frequency = InputCapture::read_frequency(id);
    EXPECT_EQ(0, frequency);

    //case with incorrect PIN
    InputCapture::turn_on(11);
    frequency = InputCapture::read_frequency(11);
    EXPECT_EQ(0, frequency);


}


TEST(InputCapture, ReadDutyCycle){


    uint8_t* PE2_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + SHM::pin_offsets[PE2]);
    uint8_t id = InputCapture::inscribe(PE2);

    uint8_t value = 80;
    *reinterpret_cast<uint8_t*>(PE2_memory+1) = value;


    //case with correct PIN and active Instance
    InputCapture::turn_on(id);
    uint8_t duty_cycle = InputCapture::read_duty_cycle(id);
    EXPECT_EQ(value, duty_cycle);

    //case with correct PIN and deactive Instance
    InputCapture::turn_off(id);
    duty_cycle = InputCapture::read_duty_cycle(id);
    EXPECT_EQ(0, duty_cycle);

    //case with incorrect PIN
    InputCapture::turn_on(11);
    duty_cycle = InputCapture::read_duty_cycle(11);
    EXPECT_EQ(0, duty_cycle);

    SharedMemory::close();
}
