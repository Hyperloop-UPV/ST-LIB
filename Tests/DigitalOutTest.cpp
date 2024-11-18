#include <gtest/gtest.h>
#include "HALALMock/Services/DigitalOutputService/DigitalOutputService.hpp"
#include "HALALMock/Services/SharedMemory/SharedMemory.hpp"


TEST(DigitalOut, Turn_On) {
    
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    uint8_t digitalout_id=DigitalOutputService::inscribe(PA0);

    DigitalOutputService::turn_on(digitalout_id);

    EmulatedPin& output_pin=SharedMemory::get_pin(PA0);
    
    EXPECT_EQ(output_pin.PinData.digital_output.state, true);

    SharedMemory::close();
}

TEST(DigitalOut, Turn_Off) {
    
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    uint8_t digitalout_id=DigitalOutputService::inscribe(PA0);

    DigitalOutputService::turn_off(digitalout_id);

    EmulatedPin& output_pin=SharedMemory::get_pin(PA0);
    
    EXPECT_EQ(output_pin.PinData.digital_output.state, false);

    SharedMemory::close();
}

TEST(DigitalOut, Set_Pin_State) {
    
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    uint8_t digitalout_id=DigitalOutputService::inscribe(PA0);
    // test ON state
    DigitalOutputService::set_pin_state(digitalout_id, PinState::ON);
    EmulatedPin& output_pin=SharedMemory::get_pin(PA0);
    EXPECT_EQ(output_pin.PinData.digital_output.state, PinState::ON);

    // test OFF state
    DigitalOutputService::set_pin_state(digitalout_id, PinState::OFF);
    output_pin=SharedMemory::get_pin(PA0);
    EXPECT_EQ(output_pin.PinData.digital_output.state, PinState::OFF);

    SharedMemory::close();
}

TEST(DigitalOut, Toggle) {
    
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    uint8_t digitalout_id=DigitalOutputService::inscribe(PA0);


    DigitalOutputService::set_pin_state(digitalout_id, PinState::ON);
    DigitalOutputService::toggle(digitalout_id);
    EmulatedPin& output_pin=SharedMemory::get_pin(PA0);
    EXPECT_EQ(output_pin.PinData.digital_output.state, false);

    SharedMemory::close();
}