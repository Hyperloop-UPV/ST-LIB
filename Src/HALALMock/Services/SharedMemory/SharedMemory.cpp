
#include "HALALMock/Services/SharedMemory/SharedMemory.hpp"
//includes to create the shared Memory in Posix
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <iostream>

uint8_t* SharedMemory::state_machine_memory = nullptr;
uint8_t* SharedMemory::state_machine_count = nullptr;

unordered_map<Pin, size_t> SharedMemory::pin_offsets= {
		{PA0, 0}, {PA1, 1}, {PA2, 2}, {PA3, 3}, {PA4, 4},
		{PA5, 5}, {PA6, 6}, {PA7, 7}, {PA8, 8}, {PA9, 9},
		{PA10, 10}, {PA11, 11}, {PA12, 12}, {PA13, 13}, 
		{PA14, 14}, {PA15, 15}, {PB0, 16}, {PB1, 17},
		{PB2, 18}, {PB3, 19}, {PB4, 20}, {PB5, 21},
		{PB6, 22}, {PB7, 23}, {PB8, 24}, {PB9, 25},
		{PB10, 26}, {PB11, 27}, {PB12, 28}, {PB13, 29},
		{PB14, 30}, {PB15, 31}, {PC0, 32}, {PC1, 33},
		{PC2, 34}, {PC3, 35}, {PC4, 36}, {PC5, 37},
		{PC6, 38}, {PC7, 39}, {PC8, 40}, {PC9, 41},
		{PC10, 42}, {PC11, 43}, {PC12, 44}, {PC13, 45},
		{PC14, 46}, {PC15, 47}, {PD0, 48}, {PD1, 49},
		{PD2, 50}, {PD3, 51}, {PD4, 52}, {PD5, 53},
		{PD6, 54}, {PD7, 55}, {PD8, 56}, {PD9, 57},
		{PD10, 58}, {PD11, 59}, {PD12, 60}, {PD13, 61},
		{PD14, 62}, {PD15, 63}, {PE0, 64}, {PE1, 65},
		{PE2, 66}, {PE3, 67}, {PE4, 68}, {PE5, 69},
		{PE6, 70}, {PE7, 71}, {PE8, 72}, {PE9, 73},
		{PE10, 74}, {PE11, 75}, {PE12, 76}, {PE13, 77},
		{PE14, 78}, {PE15, 79}, {PF0, 80}, {PF1, 81},
		{PF2, 82}, {PF3, 83}, {PF4, 84}, {PF5, 85},
		{PF6, 86}, {PF7, 87}, {PF8, 88}, {PF9, 89},
		{PF10, 90}, {PF11, 91}, {PF12, 92}, {PF13, 93},
		{PF14, 94}, {PF15, 95}, {PG0, 96}, {PG1, 97},
		{PG2, 98}, {PG3, 99}, {PG4, 100}, {PG5, 101},
		{PG6, 102}, {PG7, 103}, {PG8, 104}, {PG9, 105},
		{PG10, 106}, {PG11, 107}, {PG12, 108}, {PG13, 109},
		{PG14, 110}, {PG15, 111}, {PH0, 112}, {PH1, 113}
	};
void SharedMemory::start() {
	gpio_memory_name = SHM::gpio_memory_name;
	state_machine_memory_name = SHM::state_machine_memory_name;
	start_state_machine_memory(); // initialize the state machine shared memory
	start_gpio_shared_memory(); // initialize the gpio_shared_memory
}
void SharedMemory::start(const char* gpio_memory_name, const char* state_machine_memory_name) {
	this->gpio_memory_name = gpio_memory_name;
	this->state_machine_memory_name = state_machine_memory_name;
	start_state_machine_memory(); // initialize the state machine shared memory
	start_gpio_shared_memory(); // initialize the gpio_shared_memory
}
void SharedMemory::start_gpio_shared_memory(){
	//Create GPIO_Memory
	int shm_gpio_fd;
	//create shared memory object
	shm_gpio_fd = shm_open(gpio_memory_name, O_CREAT | O_RDWR,0660);
	if(shm_gpio_fd == -1){
		std::cout<<"Error to Open de Shared Memory";
		return;
	}
	//configure the size of the shared memory object
	if(ftruncate(shm_gpio_fd,gpio_memory_size) == -1){
		std::cout<<"Error to asssign memory to the Shared Memory";
		close(shm_gpio_fd);
		return;
	}
	//point gpio_memory to the beginning of shared_memory
	gpio_memory = static_cast<EmulatedPin*>(mmap(0, gpio_memory_size, PROT_WRITE | PROT_READ, MAP_SHARED, shm_gpio_fd, 0));
	if(gpio_memory == MAP_FAILED){
		std::cout<<"Error mapping Shared Memory";
        close(shm_gpio_fd);  // Close the descriptor if there is a problem with the mapping
        return;
	}
}
void SharedMemory::start_state_machine_memory(){
	// shared memory file descriptor
	int shm_state_machine_fd;

	// create the shared memory object
	shm_state_machine_fd=shm_open(state_machine_memory_name,O_CREAT | O_RDWR, 0660);
	if(shm_state_machine_fd==-1){
		std::cout<<"Error creating the shared memory object\n";
		std::terminate();
	}
	// configure the size of the shared memory object
	if(ftruncate(shm_state_machine_fd,state_machine_memory_size)==-1){
		std::cout<<"Error configuring the size of the shared memory object\n";
		close(shm_state_machine_fd);
		std::terminate();
	}
	// memory map the shared memory object
	state_machine_memory=static_cast<uint8_t*>(mmap(NULL,state_machine_memory_size,PROT_WRITE | PROT_READ,MAP_SHARED,shm_state_machine_fd,0));
	if(state_machine_memory==MAP_FAILED){
		std::cout<<"Error mapping the shared memory object\n";
		close(shm_state_machine_fd);
		std::terminate();
	}

	state_machine_count=&state_machine_memory[0];
	*state_machine_count=0;
}

void SharedMemory::close(){
	close_gpio_shared_memory();
	close_state_machine_memory();
}

void SharedMemory::close_state_machine_memory(){
	if (state_machine_memory!=nullptr){
		// unmap the shared memory object
		if(munmap(state_machine_memory,state_machine_memory_size)==-1){
			std::cout<<"Error unmapping the shared memory object\n";
			std::terminate();
		}

		// point the shared memory object to NULL
		state_machine_memory=nullptr;
	}
	
	if(shm_unlink(state_machine_memory_name)==-1){
		std::cout<<"Error unlinking the shared memory object\n";
		std::terminate();
	}

	if(shm_state_machine_fd !=-1 && close(shm_state_machine_fd)==-1){
		std::cout<<"Error closing the shared memory file descriptor\n";
		std::terminate();
	}
}
void SharedMemory::update_current_state(uint8_t index, uint8_t state){
	state_machine_memory[index]=state;
}

EmulatedPin &SharedMemory::get_pin(Pin pin){
    uint8_t offset;
	auto it = pin_offsets.find(pin);
	if(it != pin_offsets.end()){
		offset = it -> second;
	}else{
		std::cout<<"Pin %s doesn't exist",pin.to_string();
	}
	EmulatedPin *pin_memory = SharedMemory::gpio_memory + offset;
    return *pin_memory;
}