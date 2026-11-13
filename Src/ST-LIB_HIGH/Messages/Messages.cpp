/*
 * Messages.cpp
 * "Hello there"
 *  Created on: Nov 13, 2026
 *      Author: Aleks
 */

#include "ST-LIB_HIGH/Messages/Messages.hpp"
#include "HALAL/Services/Time/RTC.hpp"
#include "lwip/tcp.h"
#include <cstring>

namespace ST_LIB {

struct tcp_pcb* MessageADJv3Packet::tpcb = nullptr;
bool MessageADJv3Packet::is_connected = false;

MessageADJv3Packet::Package
MessageADJv3Packet::make(uint16_t id, const char* origin, const char* message) {
    size_t origin_len = strnlen(origin, 47);    // 47 symbols + 1 byte for \0 = 48 bytes as much
    size_t message_len = strnlen(message, 319); // 319 symbols + 1 byte for \0 = 320 bytes as much
    Package pack = {
        .packageSize = origin_len + message_len + sizeof(Data) + 2
    }; // adding 2 because we need to include \0 for both origin and message
    pack.data.id = id;
    pack.data.hours = 0;
    pack.data.minutes = 0;
    pack.data.seconds = 0;
    pack.data.subseconds = 0;
    memcpy(pack.payload, origin, origin_len); // copy value from origin to package's reserved memory
    pack.payload[origin_len] = '\0';          // add null terminator to the end of origin
    memcpy(
        pack.payload + origin_len + 1,
        message,
        message_len
    ); // copy valye from message to package's reserved memory
    pack.payload[origin_len + 1 + message_len] = '\0'; // add null terminator to the end of message
    return pack;
}
bool MessageADJv3Packet::tcp_client_send(MessageADJv3Packet::Package& pack) {
    if (tpcb == nullptr || !is_connected) {
        return false;
    }

#if defined(HAL_RTC_MODULE_ENABLED) && !defined(SIM_ON)
    RTCData rtc = Global_RTC::get_rtc_timestamp();
    pack.data.hours = rtc.hour;
    pack.data.minutes = rtc.minute;
    pack.data.seconds = rtc.second;
    pack.data.subseconds = static_cast<uint8_t>((rtc.counter * 256) / (RTC_MAX_COUNTER + 1));
#endif // if HAL_RTC was not enabled  all values were initialized before to 0
    err_t err = tcp_write(tpcb, &pack.data, pack.packageSize, TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK) {
        return false;
    }
    return tcp_output(tpcb) == ERR_OK;
}

} // namespace ST_LIB
