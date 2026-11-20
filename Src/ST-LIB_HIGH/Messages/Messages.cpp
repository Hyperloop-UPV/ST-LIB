/*
 * Messages.cpp
 * "Hello there"
 *  Created on: Nov 13, 2026
 *      Author: Aleks
 */

#include "ST-LIB_HIGH/Messages/Messages.hpp"
#include "ST-LIB_HIGH/Transport/PacketTransport.hpp"
#include "HALAL/Services/Time/RTC.hpp"
#include <cstring>

namespace ST_LIB {

MessagePacket::Package MessagePacket::make(uint16_t id, const char* origin, const char* message) {
    size_t origin_len =
        strnlen(origin, max_origin_size - 1); // 47 symbols + 1 byte for \0 = 48 bytes as much
    size_t message_len =
        strnlen(message, max_message_size - 1); // 319 symbols + 1 byte for \0 = 320 bytes as much
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
    ); // copy value from message to package's reserved memory
    pack.payload[origin_len + 1 + message_len] = '\0'; // add null terminator to the end of message
    return pack;
}
bool MessagePacket::tcp_client_send(MessagePacket::Package& pack) {
    if (!PacketTransport::is_connected) {
        return false;
    }

#if defined(HAL_RTC_MODULE_ENABLED) && !defined(SIM_ON)
    RTCData rtc = Global_RTC::get_rtc_timestamp();
    pack.data.hours = rtc.hour;
    pack.data.minutes = rtc.minute;
    pack.data.seconds = rtc.second;
    pack.data.subseconds = static_cast<uint8_t>((rtc.counter * 256) / (RTC_MAX_COUNTER + 1));
#endif // if HAL_RTC was not enabled  all values were initialized before to 0
    return PacketTransport::send(&pack.data, pack.packageSize);
}

bool MessagePacket::create_message(uint16_t id, const char* origin, const char* message) {
    Package pack = make(id, origin, message);
    return tcp_client_send(pack);
}

} // namespace ST_LIB
