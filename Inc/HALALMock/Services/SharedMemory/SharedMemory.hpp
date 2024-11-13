#pragma once

#include <cstdint>
#include "HALALMock/Models/PinModel/Pin.hpp"
#include "ErrorHandler/ErrorHandler.hpp"
namespace SHM{
	extern char* state_machine_memory_name;
	extern char* gpio_memory_name;

}
class SharedMemory {
   public:
	
  	static EmulatedPin *gpio_memory;
	static uint8_t* state_machine_memory;
	char* gpio_memory_name;
  	constexpr static uint8_t total_pins= 114;
	static constexpr int gpio_memory_size = total_pins * sizeof(EmulatedPin);
  
	constexpr static size_t state_machine_memory_size=16;
	static uint8_t* state_machine_count;

	static void start();
	static void start(char* name);
	static void start_gpio_shared_memory();
	static void start_state_machine_memory();

	static EmulatedPin &get_pin(Pin pin);

	static void update_current_state(uint8_t index, uint8_t state);
  
	static unordered_map<Pin, size_t> pin_offsets;
	
};
