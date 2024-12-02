#include <gtest/gtest.h>
#include "HALALMock/Services/PWM/PWM/PWM.hpp"
#include "HALALMock/Services/SharedMemory/SharedMemory.hpp"

TEST(PWM, Check_Memory_Writing){

    SharedMemory::start("GPIO_Name", "State_Machine_Name");

    for (int i = 0; i < SharedMemory::total_pins; i++) {
        ((uint8_t *)(SharedMemory::gpio_memory + i))[1]=1;
        EXPECT_EQ(((uint8_t *)(SharedMemory::gpio_memory + i))[1],1);
        ((uint8_t *)(SharedMemory::gpio_memory + i))[1]=0;
        EXPECT_EQ(((uint8_t *)(SharedMemory::gpio_memory + i))[1],0);
    }

    SharedMemory::close();

}

TEST(PWM, Turn_on){

    ErrorHandlerModel::error_triggered=0;
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    
    PWM pwm(PB14);
    EXPECT_EQ(ErrorHandlerModel::error_triggered, 0);

    pwm.turn_on();

    EmulatedPin& pwm_pin = SharedMemory::get_pin(PB14);

    EXPECT_EQ(pwm_pin.PinData.pwm.is_on, true);

    SharedMemory::close();
}

TEST(PWM, Turn_on_Memory){

    ErrorHandlerModel::error_triggered=0;
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    
    PWM pwm(PB14);
    uint8_t offset=SHM::pin_offsets[PB14];
    uint8_t* pin_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + offset);

    EXPECT_EQ(ErrorHandlerModel::error_triggered, 0);
    EXPECT_EQ(pin_memory[0],static_cast<uint8_t>(PinType::PWM));

    pwm.turn_on();

    bool* is_on=reinterpret_cast<bool*>(pin_memory+9);

    EXPECT_EQ(*is_on, true);

    SharedMemory::close();
}


TEST(PWM, Failing_turning_on){

    ErrorHandlerModel::error_triggered=0;
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    
    PWM pwm(PA0);
    EXPECT_EQ(ErrorHandlerModel::error_triggered, 1);

    pwm.turn_on();

    EmulatedPin& pwm_pin = SharedMemory::get_pin(PA0);

    EXPECT_EQ(pwm_pin.PinData.pwm.is_on, false);

    SharedMemory::close();
}

TEST(PWM,Failing_turning_on_Memory){

    ErrorHandlerModel::error_triggered=0;
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    
    PWM pwm(PA0);
    uint8_t offset=SHM::pin_offsets[PA0];
    uint8_t* pin_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + offset);

    EXPECT_EQ(ErrorHandlerModel::error_triggered, 1);
    EXPECT_EQ(pin_memory[0],static_cast<uint8_t>(PinType::NOT_USED));

    pwm.turn_on();

    bool* is_on=reinterpret_cast<bool*>(pin_memory+9);

    EXPECT_EQ(*is_on, false);

    SharedMemory::close();
}

TEST(PWM, Turn_off){
    
    ErrorHandlerModel::error_triggered=0;
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    
    PWM pwm(PB14);
    EXPECT_EQ(ErrorHandlerModel::error_triggered, 0);

    pwm.turn_on();
    pwm.turn_off();

    EmulatedPin& pwm_pin = SharedMemory::get_pin(PB14);

    EXPECT_EQ(pwm_pin.PinData.pwm.is_on, false);

    SharedMemory::close();
}

TEST(PWM, Turn_off_Memory){

    ErrorHandlerModel::error_triggered=0;
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    
    PWM pwm(PB14);
    uint8_t offset=SHM::pin_offsets[PB14];
    uint8_t* pin_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + offset);

    EXPECT_EQ(ErrorHandlerModel::error_triggered, 0);
    EXPECT_EQ(pin_memory[0],static_cast<uint8_t>(PinType::PWM));

    pwm.turn_on();
    pwm.turn_off();

    bool* is_on=reinterpret_cast<bool*>(pin_memory+9);

    EXPECT_EQ(*is_on, false);

    SharedMemory::close();
}

TEST(PWM, Duty_cycle){
    
    ErrorHandlerModel::error_triggered=0;
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    
    PWM pwm(PB14);
    EXPECT_EQ(ErrorHandlerModel::error_triggered, 0);

    pwm.turn_on();
    pwm.set_duty_cycle(50);
    float duty=pwm.get_duty_cycle();

    EmulatedPin& pwm_pin = SharedMemory::get_pin(PB14);

    EXPECT_EQ(pwm_pin.PinData.pwm.is_on, true);
    EXPECT_EQ(pwm_pin.PinData.pwm.duty_cycle, 50);
    EXPECT_EQ(duty, 50);

    SharedMemory::close();
}

TEST(PWM, Duty_cycle_Memory){

    ErrorHandlerModel::error_triggered=0;
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    
    PWM pwm(PB14);
    uint8_t offset=SHM::pin_offsets[PB14];
    uint8_t* pin_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + offset);

    EXPECT_EQ(ErrorHandlerModel::error_triggered, 0);
    EXPECT_EQ(pin_memory[0],static_cast<uint8_t>(PinType::PWM));

    pwm.turn_on();
    pwm.set_duty_cycle(50);
    float duty=pwm.get_duty_cycle();

    float* sm_duty=reinterpret_cast<float*>(pin_memory+1);
    bool* is_on=reinterpret_cast<bool*>(pin_memory+9);

    EXPECT_EQ(*is_on, true);
    EXPECT_EQ(duty, 50);
    EXPECT_EQ(*sm_duty,50);

    SharedMemory::close();
}

TEST(PWM, Frequency){
    
    ErrorHandlerModel::error_triggered=0;
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    
    PWM pwm(PB14);
    EXPECT_EQ(ErrorHandlerModel::error_triggered, 0);

    pwm.turn_on();
    pwm.set_frequency(10000);
    uint32_t freq=pwm.get_frequency();

    EmulatedPin& pwm_pin = SharedMemory::get_pin(PB14);

    EXPECT_EQ(pwm_pin.PinData.pwm.is_on, true);
    EXPECT_EQ(pwm_pin.PinData.pwm.frequency, 10000);
    EXPECT_EQ(freq, 10000);

    SharedMemory::close();
}

TEST(PWM, Frequency_Memory){

    ErrorHandlerModel::error_triggered=0;
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    
    PWM pwm(PB14);
    uint8_t offset=SHM::pin_offsets[PB14];
    uint8_t* pin_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + offset);

    EXPECT_EQ(ErrorHandlerModel::error_triggered, 0);
    EXPECT_EQ(pin_memory[0],static_cast<uint8_t>(PinType::PWM));

    pwm.turn_on();
    pwm.set_frequency(10000);
    uint32_t freq=pwm.get_frequency();

    uint32_t* sm_freq=reinterpret_cast<uint32_t*>(pin_memory+5);
    bool* is_on=reinterpret_cast<bool*>(pin_memory+9);

    EXPECT_EQ(*is_on, true);
    EXPECT_EQ(freq, 10000);
    EXPECT_EQ(*sm_freq,10000);

    SharedMemory::close();
}

TEST(PWM, Deadtime_while_on){
    
    ErrorHandlerModel::error_triggered=0;
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    
    PWM pwm(PB14);
    EXPECT_EQ(ErrorHandlerModel::error_triggered, 0);

    pwm.turn_on();
    pwm.set_dead_time(std::chrono::nanoseconds(1000000));

    EmulatedPin& pwm_pin = SharedMemory::get_pin(PB14);

    EXPECT_EQ(pwm_pin.PinData.pwm.is_on, true);
    EXPECT_EQ(pwm_pin.PinData.pwm.dead_time_ns, std::chrono::nanoseconds(0).count());
    EXPECT_EQ(ErrorHandlerModel::error_triggered, 1);
    
    SharedMemory::close();
}

TEST(PWM, Deadtime_while_on_Memory){

    ErrorHandlerModel::error_triggered=0;
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    
    PWM pwm(PB14);
    uint8_t offset=SHM::pin_offsets[PB14];
    uint8_t* pin_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + offset);

    EXPECT_EQ(ErrorHandlerModel::error_triggered, 0);
    EXPECT_EQ(pin_memory[0],static_cast<uint8_t>(PinType::PWM));

    pwm.turn_on();
    pwm.set_dead_time(std::chrono::nanoseconds(1000000));

    int64_t* sm_deadtime=reinterpret_cast<int64_t*>(pin_memory+10);
    bool* is_on=reinterpret_cast<bool*>(pin_memory+9);

    EXPECT_EQ(*is_on, true);
    EXPECT_EQ(*sm_deadtime, static_cast<int64_t>(0));
    EXPECT_EQ(ErrorHandlerModel::error_triggered, 1);

    SharedMemory::close();
}

TEST(PWM, Deadtime_while_off){
    
    ErrorHandlerModel::error_triggered=0;
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    
    PWM pwm(PB14);
    EXPECT_EQ(ErrorHandlerModel::error_triggered, 0);
    
    pwm.turn_on();
    pwm.turn_off();
    pwm.set_dead_time(std::chrono::nanoseconds(1000000));

    EmulatedPin& pwm_pin = SharedMemory::get_pin(PB14);

    EXPECT_EQ(pwm_pin.PinData.pwm.is_on, false);
    EXPECT_EQ(pwm_pin.PinData.pwm.dead_time_ns, std::chrono::nanoseconds(1000000).count());
    EXPECT_EQ(ErrorHandlerModel::error_triggered, 0);
    
    SharedMemory::close();
}

TEST(PWM, Deadtime_while_off_Memory){

    ErrorHandlerModel::error_triggered=0;
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    
    PWM pwm(PB14);
    uint8_t offset=SHM::pin_offsets[PB14];
    uint8_t* pin_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + offset);

    EXPECT_EQ(ErrorHandlerModel::error_triggered, 0);
    EXPECT_EQ(pin_memory[0],static_cast<uint8_t>(PinType::PWM));

    pwm.turn_on();
    pwm.turn_off();
    pwm.set_dead_time(std::chrono::nanoseconds(1000000));

    int64_t* sm_deadtime=reinterpret_cast<int64_t*>(pin_memory+10);
    bool* is_on=reinterpret_cast<bool*>(pin_memory+9);

    EXPECT_EQ(*is_on, false);
    EXPECT_EQ(*sm_deadtime, static_cast<int64_t>(1000000));
    EXPECT_EQ(ErrorHandlerModel::error_triggered, 0);

    SharedMemory::close();
}