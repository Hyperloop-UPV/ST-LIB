#include "HALAL/Models/MPUManager/MPUManager.hpp"

#ifdef SIM_ON
alignas(32) uint8_t mpu_manager_memory_pool[NO_CACHED_RAM_MAXIMUM_SPACE];
#else
__attribute__((section(".ram_d3_nc.legacy"))) alignas(32
) volatile uint8_t mpu_manager_memory_pool[NO_CACHED_RAM_MAXIMUM_SPACE];
#endif

volatile void* MPUManager::no_cached_ram_start = mpu_manager_memory_pool;
uint32_t MPUManager::no_cached_ram_occupied_bytes = 0;
