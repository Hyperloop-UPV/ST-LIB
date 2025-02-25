#include <gtest/gtest.h>
#include "HALALMock/Services/DigitalOutputService/DigitalOutputService.hpp"
#include "HALALMock/Services/SharedMemory/SharedMemory.hpp"


TEST(DigitalOut, Check_Memory_Writing){

    SharedMemory::start("GPIO_Name", "State_Machine_Name");

    for (int i = 0; i < SharedMemory::total_pins; i++) {
        ((uint8_t *)(SharedMemory::gpio_memory + i))[1]=1;
        EXPECT_EQ(((uint8_t *)(SharedMemory::gpio_memory + i))[1],1);
        ((uint8_t *)(SharedMemory::gpio_memory + i))[1]=0;
        EXPECT_EQ(((uint8_t *)(SharedMemory::gpio_memory + i))[1],0);
    }

    SharedMemory::close();

}

TEST(DigitalOut, Inscribe) {
    
    SharedMemory::start("GPIO_Name", "State_Machine_Name");

    uint8_t digitalout_id_1 = DigitalOutputService::inscribe(PA0);
    EXPECT_EQ(digitalout_id_1, 0);
    uint8_t* PA0_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + SHM::pin_offsets[PA0]);
    EXPECT_EQ(PA0_memory[0], static_cast<uint8_t>(PinType::DigitalOutput));

    uint8_t digitalout_id_2 = DigitalOutputService::inscribe(PA1);
    EXPECT_EQ(digitalout_id_2, 1);
    uint8_t* PA1_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + SHM::pin_offsets[PA1]);
    EXPECT_EQ(PA1_memory[0], static_cast<uint8_t>(PinType::DigitalOutput));

    uint8_t digitalout_id_3 = DigitalOutputService::inscribe(PC2);
    EXPECT_EQ(digitalout_id_3, 2);
    uint8_t* PC2_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + SHM::pin_offsets[PC2]);
    EXPECT_EQ(PC2_memory[0], static_cast<uint8_t>(PinType::DigitalOutput));

    SharedMemory::close();
}

TEST(DigitalOut, Turn_On) {
    
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    uint8_t digitalout_id=DigitalOutputService::inscribe(PA0);

    DigitalOutputService::turn_on(digitalout_id);

    EmulatedPin& output_pin=SharedMemory::get_pin(PA0);
    
    EXPECT_EQ(output_pin.PinData.digital_output.state, PinState::ON);

    SharedMemory::close();
}

TEST(DigitalOut, Turn_On_Memory) {
    
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    uint8_t digitalout_id=DigitalOutputService::inscribe(PA0);

    DigitalOutputService::turn_on(digitalout_id);

    uint8_t offset=SHM::pin_offsets[PA0];
    uint8_t* pin_memory=reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + offset);
    
    EXPECT_EQ(pin_memory[1], PinState::ON);

    SharedMemory::close();
}

TEST(DigitalOut, Turn_Off) {
    
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    uint8_t digitalout_id=DigitalOutputService::inscribe(PA0);

    DigitalOutputService::turn_on(digitalout_id);
    DigitalOutputService::turn_off(digitalout_id);

    EmulatedPin& output_pin=SharedMemory::get_pin(PA0);
    
    EXPECT_EQ(output_pin.PinData.digital_output.state, PinState::OFF);

    SharedMemory::close();
}

TEST(DigitalOut, Turn_Off_Memory) {
    
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    uint8_t digitalout_id=DigitalOutputService::inscribe(PA0);

    DigitalOutputService::turn_on(digitalout_id);
    DigitalOutputService::turn_off(digitalout_id);

    uint8_t offset=SHM::pin_offsets[PA0];
    uint8_t* pin_memory=reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + offset);
    
    EXPECT_EQ(pin_memory[1], PinState::OFF);

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

TEST(DigitalOut, Set_PinState_Memory) {
    
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    uint8_t digitalout_id=DigitalOutputService::inscribe(PA0);
    // test ON state
    DigitalOutputService::set_pin_state(digitalout_id, PinState::ON);
    uint8_t offset=SHM::pin_offsets[PA0];
    uint8_t* pin_memory=reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + offset);
    EXPECT_EQ(pin_memory[1], PinState::ON);

    // test OFF state
    DigitalOutputService::set_pin_state(digitalout_id, PinState::OFF);
    EXPECT_EQ(pin_memory[1], PinState::OFF);

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

TEST(DigitalOut, Toggle_Memory) {
    
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    uint8_t digitalout_id=DigitalOutputService::inscribe(PA0);
    uint8_t offset=SHM::pin_offsets[PA0];
    uint8_t* pin_memory=reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + offset);


    DigitalOutputService::set_pin_state(digitalout_id, PinState::ON);
    EXPECT_EQ(pin_memory[1], PinState::ON);
    DigitalOutputService::toggle(digitalout_id);
    EXPECT_EQ(pin_memory[1], PinState::OFF);

    SharedMemory::close();
}