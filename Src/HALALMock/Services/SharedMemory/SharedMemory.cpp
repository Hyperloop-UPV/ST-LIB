
#include "HALALMock/Services/SharedMemory/SharedMemory.hpp"
//includes to create the shared Memory in Posix
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
void SharedMemory::start() {
    //Create GPIO_Memory
	const char* 
	int shm_gpio_fd;
	//create shared memory object
	shm_gpio_fd = shm_open(gpio_memory_name, O_CREAT | O_RDWR,0660);
	if(shm_gpio_fd == -1){
		ErrorHandler("%s","Error to Open de Shared Memory");
		return;
	}
	//configure the size of the shared memory object
	if(ftruncate(shm_gpio_fd,gpio_memory_size) == -1){
		ErrorHandler("%s","Error to asssign memory to the Shared Memory");
		close(shm_gpio_fd);
		return;
	}
	//point gpio_memory to the beginning of shared_memory
	gpio_memory = mmap(0, gpio_memory_size, PROT_WRITE | PROT_READ, MAP_SHARED, shm_gpio_fd, 0);
	if(gpio_memory == MAP_FAILED){
		ErrorHandler("%s", "Error al mapear la memoria compartida");
        close(shm_gpio_fd);  // Close the descriptor if there is a problem with the mapping
        return;
	}
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