#include "HALAL/Models/MPUManager/MPUManager.hpp"

__attribute__((section(".mpu_ram_d3_nc.legacy"))) alignas(32
) uint8_t mpu_manager_memory_pool[NO_CACHED_RAM_MAXIMUM_SPACE];

void* MPUManager::no_cached_ram_start = mpu_manager_memory_pool;
uint32_t MPUManager::no_cached_ram_occupied_bytes = 0;
