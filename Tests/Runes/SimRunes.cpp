#include "HALAL/HALAL.hpp"

const char* SHM::gpio_memory_name = nullptr;  
const char* SHM::state_machine_memory_name = nullptr;
std::unordered_map<Pin, size_t> SHM::pin_offsets;