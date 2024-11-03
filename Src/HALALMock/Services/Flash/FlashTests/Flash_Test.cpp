/*
 * Flash_Test.cpp
 *
 *  Created on: Oct 20, 2022
 *      Author: Pablo
 */
#include "HALALMock/Services/Flash/FlashTests/Flash_Test.hpp"

#define NUMERIC_PI 		3.1415
#define NUMERIC_E  			2.7183
#define SQUARE_ROOT_OF_TWO 	1.4142

namespace FlashTest{

// 	void float_2_bytes(uint8_t * ftoa_bytes_temp, float float_variable){
// 		union {
// 		  float a;
// 		  uint8_t bytes[4];
// 		} uni;

// 		uni.a = float_variable;

// 		for (uint8_t i = 0; i < 4; i++) {
// 		  ftoa_bytes_temp[i] = uni.bytes[i];
// 		}
// 	}

// 	float bytes_2_float(uint8_t * ftoa_bytes_temp){
// 		union {
// 		  float a;
// 		  uint8_t bytes[4];
// 		} uni;

// 		for (uint8_t i = 0; i < 4; i++) {
// 			uni.bytes[i] = ftoa_bytes_temp[i];
// 		}

// 	   float float_variable =  uni.a;
// 	   return float_variable;
// 	}

// 	bool test1_writing_1_float(){
// 		uint8_t float_byte_array[4];
// 		uint32_t addr = FLASH_SECTOR4_START_ADDRESS;

// 		float_2_bytes(float_byte_array, NUMERIC_PI);
// 		return Flash::write((uint32_t *)float_byte_array, addr,(uint32_t) 1);
// 	}

// 	bool test2_reading_1_float(){
// 		uint8_t float_byte_array[4];

// 		Flash::read(FLASH_SECTOR4_START_ADDRESS, (uint32_t *)float_byte_array, 1);

// 		float result = bytes_2_float(float_byte_array);

// 		return result * 10000 == NUMERIC_PI * 10000;
// 	}

//    bool	test3_writing_multiple_float(){
// 	   uint8_t float_byte_array[4];
// 	   uint32_t addr = FLASH_SECTOR4_START_ADDRESS;
// 	   uint32_t offset1 = 0x04;
// 	   uint32_t offset2 = 0x20;

// 	   float_2_bytes(float_byte_array, NUMERIC_PI);
// 	   Flash::write((uint32_t *)float_byte_array, addr,(uint32_t) 1);

// 	   float_2_bytes(float_byte_array, NUMERIC_E);
// 	   Flash::write((uint32_t *)float_byte_array, addr + offset1,(uint32_t) 1);

// 	   float_2_bytes(float_byte_array, SQUARE_ROOT_OF_TWO);
// 	   Flash::write((uint32_t *)float_byte_array, addr + offset2,(uint32_t) 1);

// 	   Flash::read(addr, (uint32_t *)float_byte_array, 1);
// 	   float first_result = bytes_2_float(float_byte_array);
// 	   bool first_write = first_result * 10000 == NUMERIC_PI * 10000;

// 	   Flash::read(addr + offset1, (uint32_t *)float_byte_array, 1);
// 	   float second_result = bytes_2_float(float_byte_array);
// 	   bool second_write = second_result * 100000 == NUMERIC_E * 100000;

// 	   Flash::read(addr + offset2, (uint32_t *)float_byte_array, 1);
// 	   float third_result = bytes_2_float(float_byte_array);
// 	   bool third_write = third_result * 100000 == SQUARE_ROOT_OF_TWO * 100000;

// 	   return first_write && second_write && third_write;
// 	}

//    bool test4_wr_in_sector_border(){
// 	   uint32_t init_data[] = {1024, 2048, 4096, 8192};
// 	   uint32_t data_result[4];
// 	   uint32_t addr = FLASH_SECTOR5_START_ADDRESS - 0x08;

// 	   Flash::write(&init_data[0], addr,(uint32_t) 4);
// 	   Flash::read(addr, &data_result[0], 4);

// 	   for(int i = 0; i < 4; i++){
// 		  if (init_data[i] != data_result[i]) {
// 			return false;
// 		  }
// 	   }

// 	   return true;
//    }

//    bool test5_write_time_1_word(){
// 	   uint32_t data = 0xAABBCCDD;
// 	   uint32_t addr_data = FLASH_SECTOR6_START_ADDRESS - 0x04;
// 	   uint32_t addr_result = FLASH_SECTOR4_START_ADDRESS;
// 	   uint16_t offset = 0x10;
// 	   bool result;

// 	   uint32_t init = HAL_GetTick();
// 	   result = Flash::write(&data, addr_data,(uint32_t) 1);
// 	   uint32_t end = HAL_GetTick() - init;


// 	   return result && Flash::write(&end, (uint32_t)(addr_result + offset), 1);

//    }

//    bool test6_write_time_1000_word(){
// 	   uint32_t data[1000];
// 	   uint32_t addr_data = FLASH_SECTOR6_START_ADDRESS;
// 	   uint32_t addr_result = FLASH_SECTOR4_START_ADDRESS;
// 	   uint16_t offset = 0x14;
// 	   bool result;

// 	   for(int i = 0; i < 1000; i++){
// 		   data[i] = i + 1;
// 	   }

// 	   uint32_t init = HAL_GetTick();
// 	   result = Flash::write(&data[0], addr_data,(uint32_t) 1000);
// 	   uint32_t end = HAL_GetTick() - init;

// 	   return result && Flash::write(&end, addr_result + offset, 1);
//    }

//    bool test7_read_time_1000_word(uint32_t* result){
// 	   uint32_t addr_result = FLASH_SECTOR4_START_ADDRESS;
// 	   uint16_t offset = 0x18;

// 	   uint32_t init = HAL_GetTick();
// 	   Flash::read(addr_result, result, 1000);
// 	   uint32_t end = HAL_GetTick() - init;

// 	   return Flash::write(&end, addr_result + offset, 1);
//    }

//    bool test8_erase_time_1_sector(){
// 	   uint32_t addr_result = FLASH_SECTOR4_START_ADDRESS;
// 	   uint32_t sector = FLASH_SECTOR_7;
// 	   uint16_t offset = 0x1C;
// 	   bool result;

// 	   uint32_t init = HAL_GetTick();
// 	   result =  Flash::erase(sector, sector);
// 	   uint32_t end = HAL_GetTick() - init;

// 	   return result && Flash::write(&end, addr_result + offset, 1);
//    }
}
