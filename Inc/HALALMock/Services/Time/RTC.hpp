#pragma once

#include "ErrorHandler/ErrorHandler.hpp"
#include <chrono>
#include <ctime>  


	struct RTCData{
        // in Mock this var is dummy
        // just maintained for legacy
        uint16_t counter{0};
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
