#include "HALALMock/Services/SharedMemory/SharedMemory.hpp"

#include "HALALMock/Services/Logger/Logger.hpp"
// includes to create the shared Memory in Posix
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>

// initialize the static variables
EmulatedPin* SharedMemory::gpio_memory{};
uint8_t* SharedMemory::state_machine_memory = {};
char* SharedMemory::gpio_memory_name = {};
char* SharedMemory::state_machine_memory_name = {};
uint8_t* SharedMemory::state_machine_count{};
int SharedMemory::shm_gpio_fd{};
int SharedMemory::shm_state_machine_fd{};

void SharedMemory::start() {
    SharedMemory::gpio_memory_name = const_cast<char*>(SHM::gpio_memory_name);
    SharedMemory::state_machine_memory_name =
        const_cast<char*>(SHM::state_machine_memory_name);
    start_state_machine_memory();  // initialize the state machine shared memory
    start_gpio_shared_memory();    // initialize the gpio_shared_memory
}
void SharedMemory::start(const char* gpio_memory_name,
                         const char* state_machine_memory_name) {
    SharedMemory::gpio_memory_name = const_cast<char*>(gpio_memory_name);
    SharedMemory::state_machine_memory_name =
        const_cast<char*>(state_machine_memory_name);
    start_state_machine_memory();  // initialize the state machine shared memory
    start_gpio_shared_memory();    // initialize the gpio_shared_memory
}
void SharedMemory::start_gpio_shared_memory() {
    // create shared memory object
    shm_gpio_fd = shm_open(gpio_memory_name, O_CREAT | O_RDWR, 0660);
    if (shm_gpio_fd == -1) {
        LOG_ERROR("Unable to open Shared Memory");
        return;
    }
    // configure the size of the shared memory object
    if (ftruncate(shm_gpio_fd, gpio_memory_size) == -1) {
        LOG_ERROR("Unable to assign memory to Shared Memory");
        ::close(shm_gpio_fd);
        return;
    }
    // point gpio_memory to the beginning of shared_memory
    gpio_memory = static_cast<EmulatedPin*>(mmap(0, gpio_memory_size,
                                                 PROT_WRITE | PROT_READ,
                                                 MAP_SHARED, shm_gpio_fd, 0));
    if (gpio_memory == MAP_FAILED) {
        LOG_ERROR("Unable to map Shared Memory");
        ::close(shm_gpio_fd);  // Close the descriptor if there is a problem
                               // with the mapping
        return;
    }

    // clean the shared memory in case it has info from the previous execution
    memset(static_cast<void*>(gpio_memory), 0, gpio_memory_size);
}
void SharedMemory::start_state_machine_memory() {
    // create the shared memory object
    shm_state_machine_fd =
        shm_open(state_machine_memory_name, O_CREAT | O_RDWR, 0660);
    if (shm_state_machine_fd == -1) {
        LOG_ERROR("Unable to create the Shared Memory object");
        std::terminate();
    }
    // configure the size of the shared memory object
    if (ftruncate(shm_state_machine_fd, state_machine_memory_size) == -1) {
        LOG_ERROR("Unable to configure the size of the Shared Memory object");
        ::close(shm_state_machine_fd);
        std::terminate();
    }
    // memory map the shared memory object
    state_machine_memory = static_cast<uint8_t*>(
        mmap(NULL, state_machine_memory_size, PROT_WRITE | PROT_READ,
             MAP_SHARED, shm_state_machine_fd, 0));
    if (state_machine_memory == MAP_FAILED) {
        LOG_ERROR("Unable to map the Shared Memory object");
        ::close(shm_state_machine_fd);
        std::terminate();
    }

    state_machine_count = &state_machine_memory[0];
    *state_machine_count = 0;

    // clean the shared memory in case it has info from the previous execution
    memset(static_cast<void*>(state_machine_memory), 0,
           state_machine_memory_size);
}

void SharedMemory::close() {
    close_gpio_shared_memory();
    close_state_machine_memory();
}

void SharedMemory::close_gpio_shared_memory() {
    if (gpio_memory != nullptr) {
        // unmap shared memory
        if (munmap(gpio_memory, gpio_memory_size) == -1) {
            std::cout << "Error unmapping the gpio shared_memory\n";
            LOG_ERROR("Unable to unmap the gpio shared_memory");
            std::terminate();
        }
        // put the pointer to null
        gpio_memory = nullptr;
    }
    int delete_shared_gpio_memory = shm_unlink(gpio_memory_name);
    if (delete_shared_gpio_memory == -1) {
        LOG_ERROR("Unable to unlink the Shared Memory object");
        std::terminate();
    }
    if (shm_gpio_fd != -1 && ::close(shm_gpio_fd) == -1) {
        LOG_ERROR("Unable to close the file descriptor");
        std::terminate();
    }
}

void SharedMemory::close_state_machine_memory() {
    if (state_machine_memory != nullptr) {
        // unmap the shared memory object
        if (munmap(state_machine_memory, state_machine_memory_size) == -1) {
            LOG_ERROR("Unable to unmap the Shared Memory object");
            std::terminate();
        }

        // point the shared memory object to NULL
        state_machine_memory = nullptr;
    }

    if (shm_unlink(state_machine_memory_name) == -1) {
        LOG_ERROR("Unable to unlink the Shared Memory object");
        std::terminate();
    }

    if (shm_state_machine_fd != -1 && ::close(shm_state_machine_fd) == -1) {
        LOG_ERROR("Unable to close the Shared Memory file descriptor");

        std::terminate();
    }
}
void SharedMemory::update_current_state(uint8_t index, uint8_t state) {
    state_machine_memory[index] = state;
}

EmulatedPin& SharedMemory::get_pin(Pin pin) {
    auto it = SHM::pin_offsets.find(pin);
    if (it == SHM::pin_offsets.end()) {
        LOG_ERROR(std::format("Pin {} does not exist", pin.to_string()));
        std::terminate();
    }

    size_t offset = it->second;
    if (offset >= gpio_memory_size) {
        LOG_ERROR(std::format("Offset {} is out of scope", offset));
        std::terminate();
    }

    return gpio_memory[offset];
}

uint8_t SharedMemory::get_current_state(uint8_t id){
	return SharedMemory::state_machine_memory[id];
}
