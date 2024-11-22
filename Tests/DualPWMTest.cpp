#include <gtest/gtest.h>
#include "HALALMock/Services/PWM/DualPWM/DualPWM.hpp"
#include "HALALMock/Services/SharedMemory/SharedMemory.hpp"

TEST(DualPWM, Turn_on){
    
    ErrorHandlerModel::error_triggered=0;
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    
    DualPWM dualpwm(PE5,PE4);
    EXPECT_EQ(ErrorHandlerModel::error_triggered, 0);
    dualpwm.turn_on();

    EmulatedPin& dualpwm_pin = SharedMemory::get_pin(PE5);
    EmulatedPin& dualpwm_pin_negated = SharedMemory::get_pin(PE4);

    EXPECT_EQ(dualpwm_pin.PinData.dual_pwm.is_on, true);
    EXPECT_EQ(dualpwm_pin_negated.PinData.dual_pwm.is_on, true);

    SharedMemory::close();
}

TEST(DualPWM, Failing_turning_on){
    ErrorHandlerModel::error_triggered=0;
    SharedMemory::start("GPIO_Name", "State_Machine_Name");

    DualPWM dualpwm(PA0,PA1);
    EXPECT_EQ(ErrorHandlerModel::error_triggered, 1);

    dualpwm.turn_on();

    EmulatedPin& dualpwm_pin = SharedMemory::get_pin(PA0);
    EmulatedPin& dualpwm_pin_negated = SharedMemory::get_pin(PA1);

    EXPECT_EQ(dualpwm_pin.PinData.dual_pwm.is_on, false);
    EXPECT_EQ(dualpwm_pin_negated.PinData.dual_pwm.is_on, false);

    SharedMemory::close();
}

TEST(DualPWM, Turn_on_positive){
    
    ErrorHandlerModel::error_triggered=0;
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    
    DualPWM dualpwm(PE5,PE4);
    EXPECT_EQ(ErrorHandlerModel::error_triggered, 0);
    dualpwm.turn_on_positive();

    EmulatedPin& dualpwm_pin = SharedMemory::get_pin(PE5);
    EmulatedPin& dualpwm_pin_negated = SharedMemory::get_pin(PE4);

    EXPECT_EQ(dualpwm_pin.PinData.dual_pwm.is_on, true);
    EXPECT_EQ(dualpwm_pin_negated.PinData.dual_pwm.is_on, false);

    SharedMemory::close();
}

TEST(DualPWM, Turn_on_negated){
    
    ErrorHandlerModel::error_triggered=0;
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    
    DualPWM dualpwm(PE5,PE4);
    EXPECT_EQ(ErrorHandlerModel::error_triggered, 0);
    dualpwm.turn_on_negated();

    EmulatedPin& dualpwm_pin = SharedMemory::get_pin(PE5);
    EmulatedPin& dualpwm_pin_negated = SharedMemory::get_pin(PE4);

    EXPECT_EQ(dualpwm_pin.PinData.dual_pwm.is_on, false);
    EXPECT_EQ(dualpwm_pin_negated.PinData.dual_pwm.is_on, true);

    SharedMemory::close();
}

TEST(DualPWM, Turn_off){
    
    ErrorHandlerModel::error_triggered=0;
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    
    DualPWM dualpwm(PE5,PE4);
    EXPECT_EQ(ErrorHandlerModel::error_triggered, 0);
    dualpwm.turn_on();
    dualpwm.turn_off();

    EmulatedPin& dualpwm_pin = SharedMemory::get_pin(PE5);
    EmulatedPin& dualpwm_pin_negated = SharedMemory::get_pin(PE4);

    EXPECT_EQ(dualpwm_pin.PinData.dual_pwm.is_on, false);
    EXPECT_EQ(dualpwm_pin_negated.PinData.dual_pwm.is_on, false);

    SharedMemory::close();
}

TEST(DualPWM, Turn_off_positive){
    
    ErrorHandlerModel::error_triggered=0;
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    
    DualPWM dualpwm(PE5,PE4);
    EXPECT_EQ(ErrorHandlerModel::error_triggered, 0);
    dualpwm.turn_on();
    dualpwm.turn_off_positive();

    EmulatedPin& dualpwm_pin = SharedMemory::get_pin(PE5);
    EmulatedPin& dualpwm_pin_negated = SharedMemory::get_pin(PE4);

    EXPECT_EQ(dualpwm_pin.PinData.dual_pwm.is_on, false);
    EXPECT_EQ(dualpwm_pin_negated.PinData.dual_pwm.is_on, true);

    SharedMemory::close();
}

TEST(DualPWM, Turn_off_negated){
    
    ErrorHandlerModel::error_triggered=0;
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    
    DualPWM dualpwm(PE5,PE4);
    EXPECT_EQ(ErrorHandlerModel::error_triggered, 0);
    dualpwm.turn_on();
    dualpwm.turn_off_negated();

    EmulatedPin& dualpwm_pin = SharedMemory::get_pin(PE5);
    EmulatedPin& dualpwm_pin_negated = SharedMemory::get_pin(PE4);

    EXPECT_EQ(dualpwm_pin.PinData.dual_pwm.is_on, true);
    EXPECT_EQ(dualpwm_pin_negated.PinData.dual_pwm.is_on, false);

    SharedMemory::close();
}

TEST(DualPWM, Duty_cycle){
    
    ErrorHandlerModel::error_triggered=0;
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    
    DualPWM dualpwm(PE5,PE4);
    EXPECT_EQ(ErrorHandlerModel::error_triggered, 0);
    dualpwm.turn_on();
    dualpwm.set_duty_cycle(50);

    float duty=dualpwm.get_duty_cycle();
    EmulatedPin& dualpwm_pin = SharedMemory::get_pin(PE5);
    EmulatedPin& dualpwm_pin_negated = SharedMemory::get_pin(PE4);

    EXPECT_EQ(dualpwm_pin.PinData.dual_pwm.is_on, true);
    EXPECT_EQ(dualpwm_pin_negated.PinData.dual_pwm.is_on, true);
    EXPECT_EQ(dualpwm_pin.PinData.dual_pwm.duty_cycle, 50);
    EXPECT_EQ(dualpwm_pin_negated.PinData.dual_pwm.duty_cycle, 50);
    EXPECT_EQ(duty, 50);

    SharedMemory::close();
}

TEST(DualPWM, Frequency){
    
    ErrorHandlerModel::error_triggered=0;
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    
    DualPWM dualpwm(PE5,PE4);
    EXPECT_EQ(ErrorHandlerModel::error_triggered, 0);
    dualpwm.turn_on();
    dualpwm.set_frequency(10000);

    uint32_t freq=dualpwm.get_frequency();
    EmulatedPin& dualpwm_pin = SharedMemory::get_pin(PE5);
    EmulatedPin& dualpwm_pin_negated = SharedMemory::get_pin(PE4);

    EXPECT_EQ(dualpwm_pin.PinData.dual_pwm.is_on, true);
    EXPECT_EQ(dualpwm_pin_negated.PinData.dual_pwm.is_on, true);
    EXPECT_EQ(dualpwm_pin.PinData.dual_pwm.frequency, 10000);
    EXPECT_EQ(dualpwm_pin_negated.PinData.dual_pwm.frequency, 10000);
    EXPECT_EQ(freq, 10000);
    
    SharedMemory::close();
}

TEST(DualPWM, Deadtime_while_on){
    
    ErrorHandlerModel::error_triggered=0;
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    
    DualPWM dualpwm(PE5,PE4);
    EXPECT_EQ(ErrorHandlerModel::error_triggered, 0);
    dualpwm.turn_on();

    dualpwm.set_dead_time(std::chrono::nanoseconds(1000000));
    EmulatedPin& dualpwm_pin = SharedMemory::get_pin(PE5);
    EmulatedPin& dualpwm_pin_negated = SharedMemory::get_pin(PE4);

    EXPECT_EQ(dualpwm_pin.PinData.dual_pwm.is_on, true);
    EXPECT_EQ(dualpwm_pin_negated.PinData.dual_pwm.is_on, true);
    EXPECT_EQ(dualpwm_pin.PinData.dual_pwm.dead_time_ns, std::chrono::nanoseconds(0));
    EXPECT_EQ(dualpwm_pin_negated.PinData.dual_pwm.dead_time_ns, std::chrono::nanoseconds(0));
    EXPECT_EQ(ErrorHandlerModel::error_triggered, 1);
    
    SharedMemory::close();
}

TEST(DualPWM, Deadtime_while_off){
    
    ErrorHandlerModel::error_triggered=0;
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    
    DualPWM dualpwm(PE5,PE4);
    EXPECT_EQ(ErrorHandlerModel::error_triggered, 0);
    dualpwm.turn_on();
    dualpwm.turn_off();

    dualpwm.set_dead_time(std::chrono::nanoseconds(1000000));
    EmulatedPin& dualpwm_pin = SharedMemory::get_pin(PE5);
    EmulatedPin& dualpwm_pin_negated = SharedMemory::get_pin(PE4);

    EXPECT_EQ(dualpwm_pin.PinData.dual_pwm.is_on, false);
    EXPECT_EQ(dualpwm_pin_negated.PinData.dual_pwm.is_on, false);
    EXPECT_EQ(dualpwm_pin.PinData.dual_pwm.dead_time_ns, std::chrono::nanoseconds(1000000));
    EXPECT_EQ(dualpwm_pin_negated.PinData.dual_pwm.dead_time_ns, std::chrono::nanoseconds(1000000));
    EXPECT_EQ(ErrorHandlerModel::error_triggered, 0);
    
    SharedMemory::close();
}