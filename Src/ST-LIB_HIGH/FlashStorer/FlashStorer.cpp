#ifndef SIM_ON
#include "FlashStorer/FlashStorer.hpp"
#include "ErrorHandler/ErrorHandler.hpp"

vector<FlashVariable> FlashStorer::variable_list = {};
uint32_t FlashStorer::total_size = 0;

void FlashStorer::read(){
	uint32_t* data = (uint32_t*)malloc(FlashStorer::total_size);
	uint32_t number_of_words = ((FlashStorer::total_size + FLASH_32BITS_WORLD - 1) / FLASH_32BITS_WORLD); //Fast ceiling the dvision result

	Flash::read(FLASH_STORER_START_ADDRESS, (uint32_t*)data, number_of_words);

	uint32_t offset = 0;
	for(FlashVariable& v: FlashStorer::variable_list){
		memcpy(v.get_pointer(), (((uint8_t*)data) + offset), v.get_size());
		offset += v.get_size();
	}

	free(data);
}

bool FlashStorer::store_all(){

	uint32_t number_of_words = ((FlashStorer::total_size + FLASH_32BITS_WORLD - 1) / FLASH_32BITS_WORLD); //Fast ceiling the dvision result

	uint32_t* data = (uint32_t*)malloc(number_of_words * FLASH_32BITS_WORLD);
	uint32_t offset = 0;
	for(FlashVariable& v: FlashStorer::variable_list){
		memcpy((((uint8_t*)data) + offset), v.get_pointer(), v.get_size());
		offset += v.get_size();
	}

	if(not Flash::write(data, FLASH_STORER_START_ADDRESS, number_of_words)){
		return false;
	}

	free(data);

	return true;
}

#endif //SIM_ON

