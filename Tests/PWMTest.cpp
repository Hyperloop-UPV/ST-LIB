#include <gtest/gtest.h>
#include "HALALMock/Services/PWM/PWM/PWM.hpp"
#include "HALALMock/Services/SharedMemory/SharedMemory.hpp"

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

TEST(PWM, Deadtime_while_on){
    
    ErrorHandlerModel::error_triggered=0;
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    
    PWM pwm(PB14);
    EXPECT_EQ(ErrorHandlerModel::error_triggered, 0);

    pwm.turn_on();
    pwm.set_dead_time(std::chrono::nanoseconds(1000000));

    EmulatedPin& pwm_pin = SharedMemory::get_pin(PB14);

    EXPECT_EQ(pwm_pin.PinData.pwm.is_on, true);
    EXPECT_EQ(pwm_pin.PinData.pwm.dead_time_ns, std::chrono::nanoseconds(0));
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
    EXPECT_EQ(pwm_pin.PinData.pwm.dead_time_ns, std::chrono::nanoseconds(1000000));
    EXPECT_EQ(ErrorHandlerModel::error_triggered, 0);
    
    SharedMemory::close();
}