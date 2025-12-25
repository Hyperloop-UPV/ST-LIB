#include "HALAL/Models/MPUManager/MPUManager.hpp"

extern "C" uint8_t __mpu_ram_d3_nc_start[];

void* MPUManager::no_cached_ram_start = __mpu_ram_d3_nc_start;
uint32_t MPUManager::no_cached_ram_occupied_bytes = 0;
