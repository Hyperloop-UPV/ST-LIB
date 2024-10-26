/*
 * Flash_Test.cpp
 *
 *  Created on: Oct 20, 2022
 *      Author: Pablo
 */
#pragma once
#include "HALALMock/Services/Flash/Flash.hpp"
namespace FlashTest{

	bool test1_writing_1_float();

	bool test2_reading_1_float();

	bool test3_writing_multiple_float();

	bool test4_wr_in_sector_border();

	bool test5_write_time_1_word();

	bool test6_write_time_1000_word();

	bool test7_read_time_1000_word(uint32_t* result);

	bool test8_erase_time_1_sector();

}




