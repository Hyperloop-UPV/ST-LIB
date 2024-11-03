#include "HALALMock/Services/Time/RTC.hpp"

/*RTC_HandleTypeDef hrtc;
RTCData Global_RTC::global_RTC;

void Global_RTC::start_rtc(){
	RTC_TimeTypeDef sTime = { 0 };
	RTC_DateTypeDef sDate = { 0 };

	hrtc.Instance = RTC;
	hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
	hrtc.Init.AsynchPrediv = 0;
	hrtc.Init.SynchPrediv = 32767;
	hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
	hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
	hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
	hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;

	if (HAL_RTC_Init(&hrtc) != HAL_OK) {
		ErrorHandler("Error on RTC Init");
	}
	sTime.Hours = 0x0;
	sTime.Minutes = 0x0;
	sTime.Seconds = 0x0;
	sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sTime.StoreOperation = RTC_STOREOPERATION_RESET;

	if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK) {
		ErrorHandler("Error while setting time at RTC start");
	}

	sDate.WeekDay = RTC_WEEKDAY_MONDAY;
	sDate.Month = RTC_MONTH_JANUARY;
	sDate.Date = 0x1;
	sDate.Year = 23;

	if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK) {
		ErrorHandler("Error while setting date at RTC start");
	}
}


RTCData Global_RTC::get_rtc_timestamp(){
	RTCData ret;
	RTC_TimeTypeDef gTime;
	RTC_DateTypeDef gDate;
	HAL_RTC_GetTime(&hrtc, &gTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &gDate, RTC_FORMAT_BIN);
	ret.counter = gTime.SecondFraction - gTime.SubSeconds;
	ret.second = gTime.Seconds;
	ret.minute = gTime.Minutes;
	ret.hour = gTime.Hours;
	ret.day = gDate.Date;
	ret.month = gDate.Month;
	ret.year = 2000 + gDate.Year;
	return ret;
}

void Global_RTC::set_rtc_data(uint16_t counter, uint8_t second, uint8_t minute, uint8_t hour, uint8_t day, uint8_t month, uint16_t year){
	RTC_TimeTypeDef gTime;
	RTC_DateTypeDef gDate;
	gTime.SubSeconds = counter;
	gTime.Seconds = second;
	gTime.Minutes = minute;
	gTime.Hours = hour;
	gTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	gTime.StoreOperation = RTC_STOREOPERATION_RESET;
	gDate.WeekDay = 0;
	gDate.Date = day;
	gDate.Month = month;
	gDate.Year = year - 2000;
	if (HAL_RTC_SetTime(&hrtc, &gTime, RTC_FORMAT_BIN) != HAL_OK)
	{
		ErrorHandler("Error on writing Time on the RTC");
	}
	if (HAL_RTC_SetDate(&hrtc, &gDate, RTC_FORMAT_BIN) != HAL_OK)
	{
		ErrorHandler("Error on writing Date on the RTC");
	}

}
void Global_RTC::update_rtc_data(){
    global_RTC = get_rtc_timestamp();
}*/
