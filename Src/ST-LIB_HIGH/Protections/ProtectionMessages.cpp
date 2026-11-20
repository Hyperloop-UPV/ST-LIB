/*
 * ProtectionMessages.cpp
 * "Hello there"
 *  Created on: Oct 22, 2026
 *      Author: Aleks
 */
#include "ST-LIB_HIGH/Protections/ProtectionMessages.hpp"
#include "HALAL/Services/Time/RTC.hpp"
#include "ST-LIB_HIGH/Transport/PacketTransport.hpp"
#include <cstring>

namespace ST_LIB {

ProtectionPacket::Package ProtectionPacket::make(
    uint16_t protection_id,
    Protections::RuleKind prot,
    Protections::SampleEncoding sample_encoding
) {
    uint8_t numericValueSize = Protections::sample_size(sample_encoding);
    Package pack = {.packageSize = sizeof(Data) + numericValueSize, .data = {}};
    uint16_t position = (protection_id >> 13) & 0x07;
    if (position == 0) {
        position = static_cast<uint16_t>(prot) & 0x07;
    }
    uint16_t measurement_id = protection_id & 0x1FFF;
    pack.data.id = (measurement_id & 0x1FFF) | (position << 13);
    pack.data.hours = 0;
    pack.data.minutes = 0;
    pack.data.seconds = 0;
    pack.data.subseconds = 0;
    pack.data.reserved = 0;
    return pack;
}
bool ProtectionPacket::tcp_client_send(ProtectionPacket::Package& pack, const void* value) {
    memcpy(pack.value, value, pack.packageSize - sizeof(Data));
#if defined(HAL_RTC_MODULE_ENABLED) && !defined(SIM_ON)
    RTCData rtc = Global_RTC::get_rtc_timestamp();
    pack.data.hours = rtc.hour;
    pack.data.minutes = rtc.minute;
    pack.data.seconds = rtc.second;
    pack.data.subseconds = static_cast<uint8_t>((rtc.counter * 256) / (RTC_MAX_COUNTER + 1));
#endif // if HAL_RTC was not enabled  all values were initialized before to 0
    return PacketTransport::send(&pack.data, pack.packageSize);
}

} // namespace ST_LIB
