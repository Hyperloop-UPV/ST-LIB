/*
 * Flash.cpp
 *
 *  Created on: Oct 19, 2022
 *      Author: Pablo
 */

#include "HALAL/Services/Flash/Flash.hpp"

void Flash::read(uint32_t source_addr, uint32_t* result, uint32_t number_of_words){
	if (source_addr < FLASH_START_ADDRESS || source_addr > FLASH_END_ADDRESS) {
			ErrorHandler("Address out of memory when trying to read flash memory.");
			return;
	}

	HAL_FLASH_Unlock();
	uint32_t i;
	for (i = 0; i < number_of_words; i++) {
		*result = *(__IO uint32_t *)(source_addr);
		source_addr += 4;
		result++;
	}
	HAL_FLASH_Lock();
}

//TODO: Estaria muy bien optimizar el uso de ram en la escritura de múltiples sectores
bool Flash::write(uint32_t * source, uint32_t dest_addr, uint32_t number_of_words){
	if (dest_addr < FLASH_SECTOR0_START_ADDRESS || dest_addr > FLASH_END_ADDRESS) {
		ErrorHandler("Address out of memory when trying to write flash memory.");
		return false;
	}

	uint32_t start_relative_position_in_words;
	uint32_t end_relative_position_in_words;
	uint32_t buff_pos, source_pos;
	uint32_t index = 0;

	uint32_t start_sector = Flash::get_sector(dest_addr);
	uint32_t start_sector_addr = Flash::get_sector_starting_address(start_sector);
	uint32_t end_address = dest_addr + ((number_of_words * 4) - 4);
	uint32_t end_sector = Flash::get_sector(end_address);
	uint8_t number_of_sectors = end_sector - start_sector + 1;
	std::unique_ptr<uint32_t[]> buffer = std::make_unique_for_overwrite<uint32_t[]>(SECTOR_SIZE_IN_WORDS * number_of_sectors);
	start_relative_position_in_words = (dest_addr - start_sector_addr) / 4 ;
	end_relative_position_in_words = start_relative_position_in_words + number_of_words - 1;
	source_pos = 0;

	Flash::read(start_sector_addr, buffer.get(), SECTOR_SIZE_IN_WORDS * number_of_sectors);
	for (buff_pos = start_relative_position_in_words; buff_pos <= end_relative_position_in_words && source_pos < number_of_words; ++buff_pos ) {
		buffer[buff_pos] = source[source_pos];
		source_pos++;
	}

	if (not Flash::erase(start_sector, end_sector)) {
		//TODO: Exception handle (Error while erasing for writing, aborting...)
		return false;
	}

	HAL_FLASH_Unlock();
	while(index < (uint32_t)(SECTOR_SIZE_IN_WORDS * number_of_sectors) ){
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, start_sector_addr, (uint32_t)&buffer[index]) == HAL_OK) {
			start_sector_addr += 4 * FLASHWORD;
			index += FLASHWORD;
		}else{
			HAL_FLASH_Lock();
			//TODO: Exception handle (Error while writing, aborting...)
			return false;
		}
	}

	HAL_FLASH_Lock();
	return true;
}

bool Flash::erase(uint32_t startSector, uint32_t endSector){
	static FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t sectorError;

	EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
	EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
	EraseInitStruct.Sector        = startSector;
	EraseInitStruct.Banks    	  = FLASH_BANK_1;
	EraseInitStruct.NbSectors     = (endSector - startSector) + 1;

	HAL_FLASH_Unlock();

	if (HAL_FLASHEx_Erase(&EraseInitStruct, &sectorError) != HAL_OK)
	{
		//TODO: Handle the exception(Error while erasing, aborting...)
		HAL_FLASH_Lock();
		return false;
	}

	HAL_FLASH_Lock();

	return true;
}

uint32_t Flash::get_sector(uint32_t Address)
{
  uint32_t sector = FLASH_SECTOR_0;

  /* BANK 1 */
  if((Address >= 0x08000000) && (Address < 0x08020000))
  {
    sector = FLASH_SECTOR_0;
  }
  else if((Address >= 0x08020000) && (Address < 0x08040000))
  {
    sector = FLASH_SECTOR_1;
  }
  else if((Address >= 0x08040000) && (Address < 0x08060000))
  {
    sector = FLASH_SECTOR_2;
  }
  else if((Address >= 0x08060000) && (Address < 0x08080000))
  {
    sector = FLASH_SECTOR_3;
  }
  else if((Address >= 0x08080000) && (Address < 0x080A0000))
  {
    sector = FLASH_SECTOR_4;
  }
  else if((Address >= 0x080A0000) && (Address < 0x080C0000))
  {
    sector = FLASH_SECTOR_5;
  }
  else if((Address >= 0x080C0000) && (Address < 0x080E0000))
  {
    sector = FLASH_SECTOR_6;
  }
  else if((Address >= 0x080E0000) && (Address < 0x08100000))
  {
    sector = FLASH_SECTOR_7;
  }

  return sector;
}

uint32_t Flash::get_sector_starting_address(uint32_t sector){
	uint32_t address;
	switch (sector) {
		case FLASH_SECTOR_0:
			address = FLASH_SECTOR0_START_ADDRESS;
		break;
		case FLASH_SECTOR_1:
			address = FLASH_SECTOR1_START_ADDRESS;
		break;
		case FLASH_SECTOR_2:
			address = FLASH_SECTOR2_START_ADDRESS;
		break;
		case FLASH_SECTOR_3:
			address = FLASH_SECTOR3_START_ADDRESS;
		break;
		case FLASH_SECTOR_4:
			address = FLASH_SECTOR4_START_ADDRESS;
		break;
		case FLASH_SECTOR_5:
			address = FLASH_SECTOR5_START_ADDRESS;
		break;
		case FLASH_SECTOR_6:
			address = FLASH_SECTOR6_START_ADDRESS;
		break;
		case FLASH_SECTOR_7:
			address = FLASH_SECTOR7_START_ADDRESS;
		break;
		default:
			address = FLASH_SECTOR0_START_ADDRESS;
			break;
	}

	return address;
}


