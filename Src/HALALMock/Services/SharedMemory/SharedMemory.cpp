
#include "HALALMock/Services/SharedMemory/SharedMemory.hpp"
//includes to create the shared Memory in Posix
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <iostream>

EmulatedPin *SharedMemory::gpio_memory{nullptr};

uint8_t *SharedMemory::state_machine_memory{nullptr};
uint8_t *SharedMemory::state_machine_count{nullptr};

void SharedMemory::start() {
	gpio_memory_name = const_cast<char*>(SHM::gpio_memory_name);
	state_machine_memory_name = const_cast<char*>(SHM::state_machine_memory_name);
	start_state_machine_memory(); // initialize the state machine shared memory
	start_gpio_shared_memory(); // initialize the gpio_shared_memory
}
void SharedMemory::start(const char* gpio_memory_name, const char* state_machine_memory_name) {
	gpio_memory_name=const_cast<char*>(gpio_memory_name);
	state_machine_memory_name=const_cast<char*>(state_machine_memory_name);
	start_state_machine_memory(); // initialize the state machine shared memory
	start_gpio_shared_memory(); // initialize the gpio_shared_memory
}
void SharedMemory::start_gpio_shared_memory(){
	//create shared memory object
	shm_gpio_fd = shm_open(gpio_memory_name, O_CREAT | O_RDWR,0660);
	if(shm_gpio_fd == -1){
		std::cout<<"Error to Open de Shared Memory";
		return;
	}
	//configure the size of the shared memory object
	if(ftruncate(shm_gpio_fd,gpio_memory_size) == -1){
		std::cout<<"Error to asssign memory to the Shared Memory";
		::close(shm_gpio_fd);
		return;
	}
	//point gpio_memory to the beginning of shared_memory
	gpio_memory = static_cast<EmulatedPin*>(mmap(0, gpio_memory_size, PROT_WRITE | PROT_READ, MAP_SHARED, shm_gpio_fd, 0));
	if(gpio_memory == MAP_FAILED){
		std::cout<<"Error mapping Shared Memory";
        ::close(shm_gpio_fd);  // Close the descriptor if there is a problem with the mapping
        return;
	}
}
void SharedMemory::start_state_machine_memory(){

	// create the shared memory object
	shm_state_machine_fd=shm_open(state_machine_memory_name,O_CREAT | O_RDWR, 0660);
	if(shm_state_machine_fd==-1){
		std::cout<<"Error creating the shared memory object\n";
		std::terminate();
	}
	// configure the size of the shared memory object
	if(ftruncate(shm_state_machine_fd,state_machine_memory_size)==-1){
		std::cout<<"Error configuring the size of the shared memory object\n";
		::close(shm_state_machine_fd);
		std::terminate();
	}
	// memory map the shared memory object
	state_machine_memory=static_cast<uint8_t*>(mmap(NULL,state_machine_memory_size,PROT_WRITE | PROT_READ,MAP_SHARED,shm_state_machine_fd,0));
	if(state_machine_memory==MAP_FAILED){
		std::cout<<"Error mapping the shared memory object\n";
		::close(shm_state_machine_fd);
		std::terminate();
	}

	state_machine_count=&state_machine_memory[0];
	*state_machine_count=0;
}

void SharedMemory::close(){
	close_gpio_shared_memory();
	close_state_machine_memory();
}

void SharedMemory::close_gpio_shared_memory(){
	if (gpio_memory != nullptr){
		//unmap shared memory
		if(munmap(gpio_memory,state_machine_memory_size) == -1){
			std::cout<<"Error unmapping the gpio shared_memory\n";
			std::terminate();
		}
		//put the pointer to null
		gpio_memory = nullptr;
	}
	int delete_shared_gpio_memory = shm_unlink(gpio_memory_name);
	if(delete_shared_gpio_memory == -1){
		std::cout<<"Error unlinking the shared memory object\n";
		std::terminate();
	}
	if(shm_gpio_fd!=-1 && ::close(shm_gpio_fd) == -1){
		std::cout<<"Error closing the file descriptor\n";
		std::terminate();
	}
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

	if(shm_state_machine_fd !=-1 && ::close(shm_state_machine_fd)==-1){
		std::cout<<"Error closing the shared memory file descriptor\n";

		std::terminate();
	}
}
void SharedMemory::update_current_state(uint8_t index, uint8_t state){
	state_machine_memory[index]=state;
}

EmulatedPin &SharedMemory::get_pin(Pin pin){
    uint8_t offset;
	auto it = SHM::pin_offsets.find(pin);
	if(it != SHM::pin_offsets.end()){
		offset = it -> second;
	}else{
		std::cout<<"Pin %s doesn't exist",pin.to_string();
	}
	EmulatedPin *pin_memory = SharedMemory::gpio_memory + offset;
    return *pin_memory;
}