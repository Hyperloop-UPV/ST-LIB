#include "HALALMock/Services/Time/RTC.hpp"

RTCData Global_RTC::global_RTC;

void Global_RTC::start_rtc(){
	// dummy method, clock in PC is synced
	// discuss wether we want this relative to process time or 
	// to real time. for now Real Time.
}


RTCData Global_RTC::get_rtc_timestamp(){
	RTCData ret;
	auto now = std::chrono::system_clock::now();
	auto now_time_t = std::chrono::system_clock::to_time_t(now);
	std::tm* timestamp = std::localtime(&now_time_t);

	ret.second = timestamp->tm_sec;
	ret.minute = timestamp->tm_min;
	ret.hour = timestamp->tm_hour;
	ret.day = timestamp->tm_mday;
	ret.month = timestamp->tm_mon + 1;
	ret.year = 1900 + timestamp->tm_year;
	return ret;
}

void Global_RTC::set_rtc_data(uint16_t counter, uint8_t second, uint8_t minute, uint8_t hour, uint8_t day, uint8_t month, uint16_t year){
	//dummy
}
void Global_RTC::update_rtc_data(){
    global_RTC = get_rtc_timestamp();
}