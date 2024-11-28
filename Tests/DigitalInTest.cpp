#include <gtest/gtest.h>
#include "HALALMock/Services/DigitalInputService/DigitalInputService.hpp"
#include "HALALMock/Services/SharedMemory/SharedMemory.hpp"

TEST(DigitalIn, Check_Memory_Writing){

    SharedMemory::start("GPIO_Name", "State_Machine_Name");

    for (int i = 0; i < SharedMemory::total_pins; i++) {
        ((uint8_t *)(SharedMemory::gpio_memory + i))[1]=1;
        EXPECT_EQ(((uint8_t *)(SharedMemory::gpio_memory + i))[1],1);
        ((uint8_t *)(SharedMemory::gpio_memory + i))[1]=0;
        EXPECT_EQ(((uint8_t *)(SharedMemory::gpio_memory + i))[1],0);
    }

    SharedMemory::close();

}

TEST(DigitalIn, Inscribe) {
    
    SharedMemory::start("GPIO_Name", "State_Machine_Name");

    uint8_t digitalin_id_1 = DigitalInput::inscribe(PA0);
    EXPECT_EQ(digitalin_id_1, 0);
    uint8_t* PA0_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + SHM::pin_offsets[PA0]);
    EXPECT_EQ(PA0_memory[0], static_cast<uint8_t>(PinType::DigitalInput));

    uint8_t digitalin_id_2 = DigitalInput::inscribe(PA1);
    EXPECT_EQ(digitalin_id_2, 1);
    uint8_t* PA1_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + SHM::pin_offsets[PA1]);
    EXPECT_EQ(PA1_memory[0], static_cast<uint8_t>(PinType::DigitalInput));

    uint8_t digitalin_id_3 = DigitalInput::inscribe(PC2);
    EXPECT_EQ(digitalin_id_3, 2);
    uint8_t* PC2_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + SHM::pin_offsets[PC2]);
    EXPECT_EQ(PC2_memory[0], static_cast<uint8_t>(PinType::DigitalInput));

    SharedMemory::close();
}

TEST(DigitalIn, Read_Off) {
    
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    uint8_t digitalin_id = DigitalInput::inscribe(PA0);

    EmulatedPin& input_pin = SharedMemory::get_pin(PA0);
    
    input_pin.PinData.digital_input.curr_state = PinState::OFF;
    PinState state = DigitalInput::read_pin_state(digitalin_id);
    EXPECT_EQ(state, PinState::OFF);

    SharedMemory::close();
}

TEST(DigitalIn, Read_Off_Memory) {
    
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    uint8_t digitalin_id = DigitalInput::inscribe(PA0);
    uint8_t offset=SHM::pin_offsets[PA0];

    uint8_t* pin_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + offset);
    pin_memory[1] = static_cast<uint8_t>(PinState::OFF);

    PinState state=DigitalInput::read_pin_state(digitalin_id);

    EXPECT_EQ(pin_memory[1], state);

    SharedMemory::close();
}

TEST(DigitalIn, Read_On) {

    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    uint8_t digitalin_id = DigitalInput::inscribe(PA0);

    EmulatedPin& input_pin = SharedMemory::get_pin(PA0);
    
    input_pin.PinData.digital_input.curr_state = PinState::ON;
    PinState state = DigitalInput::read_pin_state(digitalin_id);
    EXPECT_EQ(state, PinState::ON);

    SharedMemory::close();
}

TEST(DigitalIn, Read_On_Memory) {
    
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    uint8_t digitalin_id = DigitalInput::inscribe(PA0);
    uint8_t offset=SHM::pin_offsets[PA0];

    uint8_t* pin_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + offset);
    pin_memory[1] = static_cast<uint8_t>(PinState::ON);

    PinState state=DigitalInput::read_pin_state(digitalin_id);

    EXPECT_EQ(pin_memory[1], state);

    SharedMemory::close();
}

TEST(DigitalIn, ReadingChange) {
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    uint8_t digitalin_id = DigitalInput::inscribe(PA0);

    EmulatedPin& input_pin = SharedMemory::get_pin(PA0);
    
    input_pin.PinData.digital_input.curr_state = PinState::OFF;
    PinState state = DigitalInput::read_pin_state(digitalin_id);
    EXPECT_EQ(state, PinState::OFF);

    input_pin.PinData.digital_input.curr_state = PinState::ON;
    state= DigitalInput::read_pin_state(digitalin_id);
    EXPECT_EQ(state, PinState::ON);

    SharedMemory::close();
}

TEST(DigitalIn, ReadingChange_Memory) {
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    uint8_t digitalin_id = DigitalInput::inscribe(PA0);
    uint8_t offset=SHM::pin_offsets[PA0];
    uint8_t* pin_memory=reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + offset);

    pin_memory[1] = static_cast<uint8_t>(PinState::OFF);
    PinState state = DigitalInput::read_pin_state(digitalin_id);
    EXPECT_EQ(state, PinState::OFF);

    pin_memory[1]= PinState::ON;
    state= DigitalInput::read_pin_state(digitalin_id);
    EXPECT_EQ(state, PinState::ON);

    SharedMemory::close();
}