/*
 * Flash.hpp
 *
 *  Created on: 19 oct. 2022
 *      Author: Pablo
 */

#pragma once
#include "stm32h7xx_hal.h"

#ifdef HAL_FLASH_MODULE_ENABLED

#include "C++Utilities/CppUtils.hpp"
#include "ErrorHandler/ErrorHandler.hpp"

#define FLASH_32BITS_WORLD ((uint8_t)4U)
#define FLASHWORD 8
#define FLASH_START_ADDRESS 0x08000000
#define FLASH_SECTOR0_START_ADDRESS 0x08000000
#define FLASH_SECTOR1_START_ADDRESS 0x08020000
#define FLASH_SECTOR2_START_ADDRESS 0x08040000
#define FLASH_SECTOR3_START_ADDRESS 0x08060000
#define FLASH_SECTOR4_START_ADDRESS 0x08080000
#define FLASH_SECTOR5_START_ADDRESS 0x080A0000
#define FLASH_SECTOR6_START_ADDRESS 0x080C0000
#define FLASH_SECTOR7_START_ADDRESS 0x080E0000
#define FLASH_END_ADDRESS 0x080FFFFF
#define SECTOR_SIZE_IN_WORDS 32768

#define SECTOR_SIZE_IN_BYTES ((uint32_t)131072U)
/*
    **IMPORTANT**
    We need to reserve memory in the linker script to avoid overwriting:
     MEMORY {
        ......
        DATA (xrw) : ORIGIN = 0x8080000, LENGTH = 512K //Allocated half of the flash
        ......
        }

    To create variables in the flash we need to first create a section:
        .user_data :
            { . = ALIGN(4);
            *(.user_data)
             . = ALIGN(4);
             } >DATA

    Then on the c script we declare:
        attribute((section(".user_data"))) const uint8_t userConfig[64]

    Then we can write on the variable using HAL:
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, (uint32_t)&userConfig[index], someData);

    More info: https://github.com/HyperloopUPV-H8/ST-LIB/wiki/All-about-Flash-module
*/

class Flash {
public:
    static void read(uint32_t source_addr, uint32_t* result, uint32_t number_of_words);
    static bool write(uint32_t* source, uint32_t dest_addr, uint32_t size_in_words);
    static bool erase(uint32_t start_sector, uint32_t end_sector);

private:
    static uint32_t get_sector(uint32_t address);
    static uint32_t get_sector_starting_address(uint32_t sector);
};

#endif
