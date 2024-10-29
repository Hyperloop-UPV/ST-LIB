#pragma once

#include "C++Utilities/CppUtils.hpp"
#include "ErrorHandler/ErrorHandler.hpp"

#define NO_CACHED_RAM_MAXIMUM_SPACE 2048

extern unsigned long _no_cached_ram_start;


class MPUManager{
public:
	static struct config{
		bool using_cache = true;
	}MPUConfig;/**< MPU configuration defined un Runes.hpp*/

	static void start(){
		MPU_Region_InitTypeDef MPU_InitStruct = {0};
		HAL_MPU_Disable();

		MPU_InitStruct.Enable = MPU_REGION_ENABLE;
		MPU_InitStruct.Number = MPU_REGION_NUMBER0;
		MPU_InitStruct.BaseAddress = 0x0;
		MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
		MPU_InitStruct.SubRegionDisable = 0x87;
		MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
		MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
		MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
		MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
		MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
		MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
		HAL_MPU_ConfigRegion(&MPU_InitStruct);
		/** Initializes and configures the Region and the memory to be protected
		*/
		MPU_InitStruct.Number = MPU_REGION_NUMBER1;
		MPU_InitStruct.BaseAddress = 0x30000000;
		MPU_InitStruct.Size = MPU_REGION_SIZE_32KB;
		MPU_InitStruct.SubRegionDisable = 0x0;
		MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
		MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
		MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
		HAL_MPU_ConfigRegion(&MPU_InitStruct);
		/** Initializes and configures the Region and the memory to be protected
		*/
		MPU_InitStruct.Number = MPU_REGION_NUMBER2;
		MPU_InitStruct.Size = MPU_REGION_SIZE_512B;
		MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
		MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
		MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
		HAL_MPU_ConfigRegion(&MPU_InitStruct);
		/** Initializes and configures the Region and the memory to be protected
		*/
		MPU_InitStruct.Number = MPU_REGION_NUMBER3;
		MPU_InitStruct.BaseAddress = 0x08000000;
		MPU_InitStruct.Size = MPU_REGION_SIZE_1MB;
		MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
		MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
		HAL_MPU_ConfigRegion(&MPU_InitStruct);

		MPU_InitStruct.Number = MPU_REGION_NUMBER4;
		MPU_InitStruct.BaseAddress = 0x38000000;
		MPU_InitStruct.Size = MPU_REGION_SIZE_16KB;
		MPU_InitStruct.SubRegionDisable = 0x0;
		MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
		MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
		MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
		HAL_MPU_ConfigRegion(&MPU_InitStruct);
		/* Enables the MPU */
		HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

		if(MPUConfig.using_cache){
			SCB_EnableICache();
			SCB_EnableDCache();
		}
	}

	static void* allocate_non_cached_memory(uint32_t size){
		void* buffer = (void*)((uint8_t*)no_cached_ram_start + no_cached_ram_occupied_bytes);
		no_cached_ram_occupied_bytes = no_cached_ram_occupied_bytes + size;
		if(no_cached_ram_occupied_bytes > NO_CACHED_RAM_MAXIMUM_SPACE){
			uint32_t excess_bytes = no_cached_ram_occupied_bytes - NO_CACHED_RAM_MAXIMUM_SPACE;
			ErrorHandler("Maximum capacity on non cached ram heap exceeded by %d",excess_bytes);
			return nullptr;
		}
		return buffer;
	}

private:
	static void* no_cached_ram_start;
	static uint32_t no_cached_ram_occupied_bytes;
};

template<typename T>
class RAMBuffer{

};
