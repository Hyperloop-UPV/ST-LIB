#include <gtest/gtest.h>
#include "HALALMock/Services/DigitalOutputService/DigitalOutputService.hpp"
#include "HALALMock/Services/SharedMemory/SharedMemory.hpp"


TEST(DigitalOut, TurnOn) {
    
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    uint8_t digitalout_id=DigitalOutputService::inscribe(PA0);

    DigitalOutputService::turn_on(digitalout_id);

    EmulatedPin output_pin=SharedMemory::get_pin(PA0);
    
    EXPECT_EQ(output_pin.PinData.digital_output.state, true);

    SharedMemory::close();
}

TEST(DigitalOut, TurnOff) {
    
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    uint8_t digitalout_id=DigitalOutputService::inscribe(PA0);

    DigitalOutputService::turn_off(digitalout_id);

    EmulatedPin output_pin=SharedMemory::get_pin(PA0);
    
    EXPECT_EQ(output_pin.PinData.digital_output.state, false);

    SharedMemory::close();
}