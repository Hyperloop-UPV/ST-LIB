#pragma once

#include <cstdint>
#include "HALALMock/Models/PinModel/Pin.hpp"
#include "ErrorHandler/ErrorHandler.hpp"
namespace SHM{
	extern const char* state_machine_memory_name;
	extern const char* gpio_memory_name;
	extern unordered_map<Pin, size_t> pin_offsets;

}
class SharedMemory {
   public:
  	static EmulatedPin *gpio_memory;
	static uint8_t* state_machine_memory;
	static char* gpio_memory_name;
	static char* state_machine_memory_name;
  	static constexpr  uint8_t total_pins= 114;
	static constexpr size_t gpio_memory_size = total_pins * sizeof(EmulatedPin);
	static constexpr size_t state_machine_memory_size=16;
	static uint8_t* state_machine_count;
	//descriptors
	static int shm_gpio_fd;
	static int shm_state_machine_fd;
	
	static void start();
	static void start(const char* gp_memory_name,const char* sm_memory_name);
	static void close();
	static EmulatedPin &get_pin(Pin pin);

	static void update_current_state(uint8_t index, uint8_t state);
  

	static unordered_map<Pin, size_t> pin_offsets;
	private:
		//Private to avoid bad use of this functions from the user
		static void start_gpio_shared_memory();
		static void start_state_machine_memory();
		static void close_gpio_shared_memory();
		static void close_state_machine_memory();
};
