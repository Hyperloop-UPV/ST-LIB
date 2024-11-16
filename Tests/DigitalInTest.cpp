#include <gtest/gtest.h>
#include "HALALMock/Services/DigitalInputService/DigitalInputService.hpp"
#include "HALALMock/Services/SharedMemory/SharedMemory.hpp"

TEST(DigitalIn, Read_Off) {
    
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    uint8_t digitalin_id = DigitalInput::inscribe(PA0);

    EmulatedPin& input_pin = SharedMemory::get_pin(PA0);
    
    input_pin.PinData.digital_input.curr_state = PinState::OFF;
    PinState state = DigitalInput::read_pin_state(digitalin_id);
    EXPECT_EQ(state, PinState::OFF);

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