#include <gtest/gtest.h>
#include "HALALMock/Services/EXTI/EXTI.hpp"
#include "HALALMock/Services/SharedMemory/SharedMemory.hpp"

bool enter_on_callback_1 = false;
bool enter_on_callback_2 = false;

void callback_target_1()
{
    enter_on_callback_1 = true;
}

void callback_target_2()
{
    enter_on_callback_2 = true;
}

void test_interrupts_1(uint8_t* pin1, uint8_t* pin2)
{

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::unique_lock<std::mutex> lock(ExternalInterrupt::mutex);
    *reinterpret_cast<bool*>(pin1+6) = PinState::ON;
    lock.unlock();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    lock.lock();
    *reinterpret_cast<bool*>(pin2+6) = PinState::ON;
    lock.unlock();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
}

#define EXTI1_IRQn 33
#define EXTI2_IRQn 16

map<uint16_t, ExternalInterrupt::Instance> ExternalInterrupt::instances = {
    {PE0.gpio_pin, Instance(EXTI1_IRQn)}, {PE1.gpio_pin, Instance(EXTI2_IRQn)}, {PA0.gpio_pin, Instance(EXTI2_IRQn)}, 
};

static uint8_t id1, id2 = 0;

TEST(EXTITest, Inscribe){

    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    uint8_t* PE0_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + SHM::pin_offsets[PE0]);
    uint8_t* PE7_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + SHM::pin_offsets[PE7]);
    uint8_t* PE1_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + SHM::pin_offsets[PE1]);

    //try inscribe correct PIN
    id1 = ExternalInterrupt::inscribe(PE0, callback_target_1, TRIGGER::BOTH_EDGES);
    EXPECT_EQ(PE0_memory[0], static_cast<uint8_t>(PinType::EXTIPin));
    EXPECT_EQ(id1, 1);
    EXPECT_EQ(ExternalInterrupt::id_counter, 1);

    //try inscribe incorrect PIN
    uint8_t id = ExternalInterrupt::inscribe(PE7, callback_target_2, TRIGGER::BOTH_EDGES);
    EXPECT_EQ(PE7_memory[0], static_cast<uint8_t>(PinType::NOT_USED));
    EXPECT_EQ(id, 0);
    EXPECT_EQ(ExternalInterrupt::id_counter, 1);

    //try inscribe another PIN
    id2 = ExternalInterrupt::inscribe(PE1, callback_target_2, TRIGGER::BOTH_EDGES);
    EXPECT_EQ(PE1_memory[0], static_cast<uint8_t>(PinType::EXTIPin));
    EXPECT_EQ(id2, 2);
    EXPECT_EQ(ExternalInterrupt::id_counter, 2);

    SharedMemory::close();
}

TEST(EXTITest, Start_Stop){

    SharedMemory::start("GPIO_Name", "State_Machine_Name");

    ASSERT_FALSE(ExternalInterrupt::is_running);


    ExternalInterrupt::start();

    ASSERT_TRUE(ExternalInterrupt::is_running);

    ExternalInterrupt::stop();

    ASSERT_FALSE(ExternalInterrupt::is_running);

    SharedMemory::close();
}

TEST(EXTITest, TurnOn){

    SharedMemory::start("GPIO_Name", "State_Machine_Name");

    uint8_t* pin_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + SHM::pin_offsets[PE0]);

    *reinterpret_cast<PinType*>(pin_memory) = PinType::EXTIPin; //Just for testing, SharedMemory is reinit in each test

    ASSERT_FALSE(*reinterpret_cast<bool*>(pin_memory+5));

    ExternalInterrupt::turn_on(id1);

    ASSERT_TRUE(*reinterpret_cast<bool*>(pin_memory+5));


    SharedMemory::close();
}


TEST(EXTITest, get_pin_value){

    SharedMemory::start("GPIO_Name", "State_Machine_Name");

    uint8_t* pin_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + SHM::pin_offsets[PE1]);

    *reinterpret_cast<PinType*>(pin_memory) = PinType::EXTIPin; //Just for testing, SharedMemory is reinit in each test
    *reinterpret_cast<bool*>(pin_memory+6) = PinState::OFF;

    bool signal = ExternalInterrupt::get_pin_value(id2);
    ASSERT_FALSE(signal);
    *reinterpret_cast<bool*>(pin_memory+6) = PinState::ON;

    signal = ExternalInterrupt::get_pin_value(id2);
    ASSERT_TRUE(signal);
    SharedMemory::close();
}


TEST(EXTITest, handle_interrupt_1){

    SharedMemory::start("GPIO_Name", "State_Machine_Name");
    ExternalInterrupt::turn_on(id1);
    ExternalInterrupt::turn_on(id2);
    uint8_t* PE0_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + SHM::pin_offsets[PE0]);
    *reinterpret_cast<bool*>(PE0_memory+6) = PinState::ON;

    uint8_t* PE1_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + SHM::pin_offsets[PE1]);
    *reinterpret_cast<bool*>(PE1_memory+6) = PinState::ON;

    enter_on_callback_1 = false;
    enter_on_callback_2 = false;

    ASSERT_TRUE(*(ExternalInterrupt::instances[id1].trigger_signal));
    ASSERT_FALSE(enter_on_callback_1);
    ASSERT_FALSE(enter_on_callback_2);
    ExternalInterrupt::start();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    ExternalInterrupt::stop();
    //check if modify global variables
    ASSERT_TRUE(enter_on_callback_1);
    ASSERT_TRUE(enter_on_callback_2);

    SharedMemory::close();
}

TEST(EXTITest, handle_interrupt_concurrece){

    SharedMemory::start("GPIO_Name", "State_Machine_Name");
     uint8_t* PE0_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + SHM::pin_offsets[PE0]);
    *reinterpret_cast<bool*>(PE0_memory+6) = PinState::OFF;

    uint8_t* PE1_memory = reinterpret_cast<uint8_t*>(SharedMemory::gpio_memory + SHM::pin_offsets[PE1]);
    *reinterpret_cast<bool*>(PE1_memory+6) = PinState::OFF;

    enter_on_callback_1 = false;
    enter_on_callback_2 = false;

    ASSERT_FALSE(enter_on_callback_1);
    ASSERT_FALSE(enter_on_callback_2);
    ExternalInterrupt::start();
    
    //std::thread sim_interrupts = std::thread([&](){test_interrupts_1(pin1, pin2);});

    std::thread sim_interrupts(test_interrupts_1, std::ref(PE1_memory), std::ref(PE0_memory));
    sim_interrupts.join();
    ExternalInterrupt::stop();
    //check if modify global variables
    ASSERT_TRUE(enter_on_callback_1);
    ASSERT_TRUE(enter_on_callback_2);

    SharedMemory::close();
}