
#include "HALALMock/Services/SharedMemory/SharedMemory.hpp"
//includes to create the shared Memory in Posix
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

void SharedMemory::start() {
    // TODO: create / open shared memory
}

std::string SharedMemory::generate_shared_memory_name(){
	return "SharedMemory_emulated_state_machine_"+std::to_string(getpid());
}

void SharedMemory::start_emulated_state_machine(){
	// shared memory file descriptor
	int shm_state_machine_fd;
	emulated_state_machine_name=generate_shared_memory_name();

	// create the shared memory object
	shm_state_machine_fd=shm_open(emulated_state_machine_name.c_str(),O_CREAT | O_RDWR, 0666);
	if(shm_state_machine_fd==-1){
		ErrorHandler("%s","Error creating the shared memory object");
		return;
	}
	// configure the size of the shared memory object
	if(ftruncate(shm_state_machine_fd,emulated_state_machine_size)==-1){
		ErrorHandler("%s","Error configuring the size of the shared memory object");
		close(shm_state_machine_fd);
		return;
	}
	// memory map the shared memory object
	emulated_state_machine=mmap(NULL,emulated_state_machine_size,PROT_WRITE | PROT_READ,MAP_SHARED,shm_state_machine_fd,0);
	if(emulated_state_machine==MAP_FAILED){
		ErrorHandler("%s","Error mapping the shared memory object");
		close(shm_state_machine_fd);
		return;
	}

	// cast of the void pointer that the shared memory object returns to a uint8_t pointer
	state_machine_sm=static_cast<uint8_t*>(emulated_state_machine);
	// initialize the shared memory counter by setting it to 0, when a new state machine is created it will increment
	state_machine_sm[0]=0;
	
}

void SharedMemory::update_state_machine_counter(uint8_t counter){
	state_machine_sm[0]=counter;
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