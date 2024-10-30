
#include "HALALMock/Services/SharedMemory/SharedMemory.hpp"
//includes to create the shared Memory in Posix
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

void SharedMemory::start() {
    start_state_machine_memory();
}

std::string SharedMemory::generate_shared_memory_name(){
	return "SharedMemory_emulated_state_machine_"+std::to_string(getpid());
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

	state_machine_memory[0]=0; //this byte will act as a state machine counter
}

void SharedMemory::update_current_state(uint8_t index, uint8_t state){
	state_machine_sm[index]=state;
}

EmulatedPin &SharedMemory::get_pin(Pin pin){
    uint8_t offset;
	auto it = pin_offsets.find(pin);
	if(it != pin_offsets.end()){
		offset = it -> second;
	}else{
		ErrorHandler("Pin %s doesn't exist",pin.to_string());
	}
	EmulatedPin *pin_memory = SharedMemory::gpio_memory + offset;
    return *pin_memory;
}