#include "HALALMock/Models/MPUManager/MPUManager.hpp"

void* MPUManager::no_cached_ram_start = (void*)&_no_cached_ram_start;
uint32_t MPUManager::no_cached_ram_occupied_bytes = 0;
