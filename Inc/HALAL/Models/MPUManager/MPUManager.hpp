#pragma once

#include "C++Utilities/CppUtils.hpp"
#include "ErrorHandler/ErrorHandler.hpp"

#define NO_CACHED_RAM_MAXIMUM_SPACE 2048

class MPUManager{
public:
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
