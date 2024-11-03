#pragma once

//#include "stm32h7xx_hal.h"
#include "ErrorHandler/ErrorHandler.hpp"


#define RTC_MAX_COUNTER 32767

#ifdef HAL_RTC_MODULE_ENABLED

	struct RTCData{
		uint16_t counter;
		uint8_t second;
		uint8_t minute;
		uint8_t hour;
		uint8_t day;
		uint8_t month;
		uint16_t year;
	};
    
    class Global_RTC{
    public:
    
        static RTCData global_RTC;
        static void start_rtc();
        static void update_rtc_data();
        static RTCData get_rtc_timestamp();
        static void set_rtc_data(uint16_t counter, uint8_t second, uint8_t minute, uint8_t hour, uint8_t day, uint8_t month, uint16_t year);
        };



#endif