#include <gtest/gtest.h>
#include "HALALMock/Services/PWM/DualPWM/DualPWM.hpp"
#include "HALALMock/Services/SharedMemory/SharedMemory.hpp"

TEST(DualPWM, Check_Memory_Writing){

    SharedMemory::start("GPIO_Name", "State_Machine_Name");

    for (int i = 0; i < SharedMemory::total_pins; i++) {
        ((uint8_t *)(SharedMemory::gpio_memory + i))[1]=1;
        EXPECT_EQ(((uint8_t *)(SharedMemory::gpio_memory + i))[1],1);
        ((uint8_t *)(SharedMemory::gpio_memory + i))[1]=0;
        EXPECT_EQ(((uint8_t *)(SharedMemory::gpio_memory + i))[1],0);
    }

    SharedMemory::close();

}

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

TEST(DualPWM, Turn_on_Memory){

    ErrorHandlerModel::error_triggered=0;
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    
    DualPWM dualpwm(PE5,PE4);
    uint8_t offset_PE5=SHM::pin_offsets[PE5];
    uint8_t offset_PE4=SHM::pin_offsets[PE4];
    uint8_t* pin_memory_PE5 = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + offset_PE5);
    uint8_t* pin_memory_PE4 = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + offset_PE4);

    EXPECT_EQ(ErrorHandlerModel::error_triggered, 0);
    EXPECT_EQ(pin_memory_PE5[0],static_cast<uint8_t>(PinType::DualPWM));
    EXPECT_EQ(pin_memory_PE4[0],static_cast<uint8_t>(PinType::DualPWM));

    dualpwm.turn_on();

    bool* is_on_PE5=reinterpret_cast<bool*>(pin_memory_PE5+9);
    bool* is_on_PE4=reinterpret_cast<bool*>(pin_memory_PE4+9);

    EXPECT_EQ(*is_on_PE5, true);
    EXPECT_EQ(*is_on_PE4, true);

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

TEST(DualPWM, Failing_turning_on_Memory){

    ErrorHandlerModel::error_triggered=0;
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    
    DualPWM dualpwm(PA0,PA1);
    uint8_t offset_PA0=SHM::pin_offsets[PA0];
    uint8_t offset_PA1=SHM::pin_offsets[PA1];
    uint8_t* pin_memory_PA0 = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + offset_PA0);
    uint8_t* pin_memory_PA1 = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + offset_PA1);

    EXPECT_EQ(ErrorHandlerModel::error_triggered, 1);
    EXPECT_EQ(pin_memory_PA0[0],static_cast<uint8_t>(PinType::NOT_USED));
    EXPECT_EQ(pin_memory_PA1[0],static_cast<uint8_t>(PinType::NOT_USED));

    dualpwm.turn_on();

    bool* is_on_PA0=reinterpret_cast<bool*>(pin_memory_PA0+9);
    bool* is_on_PA1=reinterpret_cast<bool*>(pin_memory_PA1+9);

    EXPECT_EQ(*is_on_PA0, false);
    EXPECT_EQ(*is_on_PA1, false);

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

TEST(DualPWM, Turn_on_positive_Memory){

    ErrorHandlerModel::error_triggered=0;
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    
    DualPWM dualpwm(PE5,PE4);
    uint8_t offset_PE5=SHM::pin_offsets[PE5];
    uint8_t offset_PE4=SHM::pin_offsets[PE4];
    uint8_t* pin_memory_PE5 = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + offset_PE5);
    uint8_t* pin_memory_PE4 = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + offset_PE4);

    EXPECT_EQ(ErrorHandlerModel::error_triggered, 0);
    EXPECT_EQ(pin_memory_PE5[0],static_cast<uint8_t>(PinType::DualPWM));
    EXPECT_EQ(pin_memory_PE4[0],static_cast<uint8_t>(PinType::DualPWM));

    dualpwm.turn_on_positive();

    bool* is_on_PE5=reinterpret_cast<bool*>(pin_memory_PE5+9);
    bool* is_on_PE4=reinterpret_cast<bool*>(pin_memory_PE4+9);

    EXPECT_EQ(*is_on_PE5, true);
    EXPECT_EQ(*is_on_PE4, false);

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

TEST(DualPWM, Turn_on_negated_Memory){

    ErrorHandlerModel::error_triggered=0;
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    
    DualPWM dualpwm(PE5,PE4);
    uint8_t offset_PE5=SHM::pin_offsets[PE5];
    uint8_t offset_PE4=SHM::pin_offsets[PE4];
    uint8_t* pin_memory_PE5 = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + offset_PE5);
    uint8_t* pin_memory_PE4 = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + offset_PE4);

    EXPECT_EQ(ErrorHandlerModel::error_triggered, 0);
    EXPECT_EQ(pin_memory_PE5[0],static_cast<uint8_t>(PinType::DualPWM));
    EXPECT_EQ(pin_memory_PE4[0],static_cast<uint8_t>(PinType::DualPWM));

    dualpwm.turn_on_negated();

    bool* is_on_PE5=reinterpret_cast<bool*>(pin_memory_PE5+9);
    bool* is_on_PE4=reinterpret_cast<bool*>(pin_memory_PE4+9);

    EXPECT_EQ(*is_on_PE5, false);
    EXPECT_EQ(*is_on_PE4, true);

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

TEST(DualPWM, Turn_off_Memory){

    ErrorHandlerModel::error_triggered=0;
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    
    DualPWM dualpwm(PE5,PE4);
    uint8_t offset_PE5=SHM::pin_offsets[PE5];
    uint8_t offset_PE4=SHM::pin_offsets[PE4];
    uint8_t* pin_memory_PE5 = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + offset_PE5);
    uint8_t* pin_memory_PE4 = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + offset_PE4);

    EXPECT_EQ(ErrorHandlerModel::error_triggered, 0);
    EXPECT_EQ(pin_memory_PE5[0],static_cast<uint8_t>(PinType::DualPWM));
    EXPECT_EQ(pin_memory_PE4[0],static_cast<uint8_t>(PinType::DualPWM));

    dualpwm.turn_on();
    dualpwm.turn_off();

    bool* is_on_PE5=reinterpret_cast<bool*>(pin_memory_PE5+9);
    bool* is_on_PE4=reinterpret_cast<bool*>(pin_memory_PE4+9);

    EXPECT_EQ(*is_on_PE5, false);
    EXPECT_EQ(*is_on_PE4, false);

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

TEST(DualPWM, Turn_off_positive_Memory){

    ErrorHandlerModel::error_triggered=0;
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    
    DualPWM dualpwm(PE5,PE4);
    uint8_t offset_PE5=SHM::pin_offsets[PE5];
    uint8_t offset_PE4=SHM::pin_offsets[PE4];
    uint8_t* pin_memory_PE5 = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + offset_PE5);
    uint8_t* pin_memory_PE4 = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + offset_PE4);

    EXPECT_EQ(ErrorHandlerModel::error_triggered, 0);
    EXPECT_EQ(pin_memory_PE5[0],static_cast<uint8_t>(PinType::DualPWM));
    EXPECT_EQ(pin_memory_PE4[0],static_cast<uint8_t>(PinType::DualPWM));

    dualpwm.turn_on();
    dualpwm.turn_off_positive();

    bool* is_on_PE5=reinterpret_cast<bool*>(pin_memory_PE5+9);
    bool* is_on_PE4=reinterpret_cast<bool*>(pin_memory_PE4+9);

    EXPECT_EQ(*is_on_PE5, false);
    EXPECT_EQ(*is_on_PE4, true);

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

TEST(DualPWM, Turn_off_negated_Memory){

    ErrorHandlerModel::error_triggered=0;
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    
    DualPWM dualpwm(PE5,PE4);
    uint8_t offset_PE5=SHM::pin_offsets[PE5];
    uint8_t offset_PE4=SHM::pin_offsets[PE4];
    uint8_t* pin_memory_PE5 = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + offset_PE5);
    uint8_t* pin_memory_PE4 = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + offset_PE4);

    EXPECT_EQ(ErrorHandlerModel::error_triggered, 0);
    EXPECT_EQ(pin_memory_PE5[0],static_cast<uint8_t>(PinType::DualPWM));
    EXPECT_EQ(pin_memory_PE4[0],static_cast<uint8_t>(PinType::DualPWM));

    dualpwm.turn_on();
    dualpwm.turn_off_negated();

    bool* is_on_PE5=reinterpret_cast<bool*>(pin_memory_PE5+9);
    bool* is_on_PE4=reinterpret_cast<bool*>(pin_memory_PE4+9);

    EXPECT_EQ(*is_on_PE5, true);
    EXPECT_EQ(*is_on_PE4, false);

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

TEST(DualPWM, Duty_cycle_Memory){
    
    ErrorHandlerModel::error_triggered=0;
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    
    DualPWM dualpwm(PE5,PE4);
    uint8_t offset_PE5=SHM::pin_offsets[PE5];
    uint8_t offset_PE4=SHM::pin_offsets[PE4];
    uint8_t* pin_memory_PE5 = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + offset_PE5);
    uint8_t* pin_memory_PE4 = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + offset_PE4);

    EXPECT_EQ(ErrorHandlerModel::error_triggered, 0);
    EXPECT_EQ(pin_memory_PE5[0],static_cast<uint8_t>(PinType::DualPWM));
    EXPECT_EQ(pin_memory_PE4[0],static_cast<uint8_t>(PinType::DualPWM));

    dualpwm.turn_on();
    dualpwm.set_duty_cycle(50);
    float duty=dualpwm.get_duty_cycle();

    float* sm_duty_PE5=reinterpret_cast<float*>(pin_memory_PE5+1);
    float* sm_duty_PE4=reinterpret_cast<float*>(pin_memory_PE4+1);
    bool* is_on_PE5=reinterpret_cast<bool*>(pin_memory_PE5+9);
    bool* is_on_PE4=reinterpret_cast<bool*>(pin_memory_PE4+9);

    EXPECT_EQ(*is_on_PE5, true);
    EXPECT_EQ(*is_on_PE4, true);
    EXPECT_EQ(duty, 50);
    EXPECT_EQ(*sm_duty_PE5,50);
    EXPECT_EQ(*sm_duty_PE4,50);

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

TEST(DualPWM, Frequency_Memory){
    
    ErrorHandlerModel::error_triggered=0;
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    
    DualPWM dualpwm(PE5,PE4);
    uint8_t offset_PE5=SHM::pin_offsets[PE5];
    uint8_t offset_PE4=SHM::pin_offsets[PE4];
    uint8_t* pin_memory_PE5 = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + offset_PE5);
    uint8_t* pin_memory_PE4 = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + offset_PE4);

    EXPECT_EQ(ErrorHandlerModel::error_triggered, 0);
    EXPECT_EQ(pin_memory_PE5[0],static_cast<uint8_t>(PinType::DualPWM));
    EXPECT_EQ(pin_memory_PE4[0],static_cast<uint8_t>(PinType::DualPWM));

    dualpwm.turn_on();
    dualpwm.set_frequency(10000);
    uint32_t freq=dualpwm.get_frequency();

    uint32_t* sm_freq_PE5=reinterpret_cast<uint32_t*>(pin_memory_PE5+5);
    uint32_t* sm_freq_PE4=reinterpret_cast<uint32_t*>(pin_memory_PE4+5);
    bool* is_on_PE5=reinterpret_cast<bool*>(pin_memory_PE5+9);
    bool* is_on_PE4=reinterpret_cast<bool*>(pin_memory_PE4+9);

    EXPECT_EQ(*is_on_PE5, true);
    EXPECT_EQ(*is_on_PE4, true);
    EXPECT_EQ(freq, 10000);
    EXPECT_EQ(*sm_freq_PE5,10000);
    EXPECT_EQ(*sm_freq_PE4,10000);

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
    EXPECT_EQ(dualpwm_pin.PinData.dual_pwm.dead_time_ns, std::chrono::nanoseconds(0).count());
    EXPECT_EQ(dualpwm_pin_negated.PinData.dual_pwm.dead_time_ns, std::chrono::nanoseconds(0).count());
    EXPECT_EQ(ErrorHandlerModel::error_triggered, 1);
    
    SharedMemory::close();
}

TEST(DualPWM, Deadtime_while_on_Memory){
    
    ErrorHandlerModel::error_triggered=0;
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    
    DualPWM dualpwm(PE5,PE4);
    uint8_t offset_PE5=SHM::pin_offsets[PE5];
    uint8_t offset_PE4=SHM::pin_offsets[PE4];
    uint8_t* pin_memory_PE5 = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + offset_PE5);
    uint8_t* pin_memory_PE4 = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + offset_PE4);

    EXPECT_EQ(ErrorHandlerModel::error_triggered, 0);
    EXPECT_EQ(pin_memory_PE5[0],static_cast<uint8_t>(PinType::DualPWM));
    EXPECT_EQ(pin_memory_PE4[0],static_cast<uint8_t>(PinType::DualPWM));

    dualpwm.turn_on();
    dualpwm.set_dead_time(std::chrono::nanoseconds(1000000));

    int64_t* sm_deadtime_PE5=reinterpret_cast<int64_t*>(pin_memory_PE5+10);
    int64_t* sm_deadtime_PE4=reinterpret_cast<int64_t*>(pin_memory_PE4+10);
    bool* is_on_PE5=reinterpret_cast<bool*>(pin_memory_PE5+9);
    bool* is_on_PE4=reinterpret_cast<bool*>(pin_memory_PE4+9);

    EXPECT_EQ(*is_on_PE5, true);
    EXPECT_EQ(*is_on_PE4, true);
    EXPECT_EQ(*sm_deadtime_PE5,static_cast<int64_t>(0));
    EXPECT_EQ(*sm_deadtime_PE4,static_cast<int64_t>(0));
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
    EXPECT_EQ(dualpwm_pin.PinData.dual_pwm.dead_time_ns, std::chrono::nanoseconds(1000000).count());
    EXPECT_EQ(dualpwm_pin_negated.PinData.dual_pwm.dead_time_ns, std::chrono::nanoseconds(1000000).count());
    EXPECT_EQ(ErrorHandlerModel::error_triggered, 0);
    
    SharedMemory::close();
}

TEST(DualPWM, Deadtime_while_off_Memory){
    
    ErrorHandlerModel::error_triggered=0;
    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    
    DualPWM dualpwm(PE5,PE4);
    uint8_t offset_PE5=SHM::pin_offsets[PE5];
    uint8_t offset_PE4=SHM::pin_offsets[PE4];
    uint8_t* pin_memory_PE5 = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + offset_PE5);
    uint8_t* pin_memory_PE4 = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + offset_PE4);

    EXPECT_EQ(ErrorHandlerModel::error_triggered, 0);
    EXPECT_EQ(pin_memory_PE5[0],static_cast<uint8_t>(PinType::DualPWM));
    EXPECT_EQ(pin_memory_PE4[0],static_cast<uint8_t>(PinType::DualPWM));

    dualpwm.turn_on();
    dualpwm.turn_off();
    dualpwm.set_dead_time(std::chrono::nanoseconds(1000000));

    int64_t* sm_deadtime_PE5=reinterpret_cast<int64_t*>(pin_memory_PE5+10);
    int64_t* sm_deadtime_PE4=reinterpret_cast<int64_t*>(pin_memory_PE4+10);
    bool* is_on_PE5=reinterpret_cast<bool*>(pin_memory_PE5+9);
    bool* is_on_PE4=reinterpret_cast<bool*>(pin_memory_PE4+9);

    EXPECT_EQ(*is_on_PE5, false);
    EXPECT_EQ(*is_on_PE4, false);
    EXPECT_EQ(*sm_deadtime_PE5,static_cast<int64_t>(1000000));
    EXPECT_EQ(*sm_deadtime_PE4,static_cast<int64_t>(1000000));
    EXPECT_EQ(ErrorHandlerModel::error_triggered, 0);

    SharedMemory::close();
}