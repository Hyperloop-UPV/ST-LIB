/*
 * ProtectionMessages.cpp
 * "Hello there"
 *  Created on: Oct 22, 2026
 *      Author: Aleks
 */
#include "ST-LIB_HIGH/Protections/ProtectionMessages.hpp"
#include "HALAL/Services/Time/RTC.hpp"
#include "lwip/tcp.h"
#include <cstring>

namespace ST_LIB {

struct tcp_pcb* ProtectionADJv3Packet::tpcb = nullptr;
bool ProtectionADJv3Packet::is_connected = false;

ProtectionADJv3Packet::Package ProtectionADJv3Packet::make(
    uint16_t protection_id,
    Protections::RuleKind prot,
    Protections::SampleEncoding sample_encoding
) {
    uint8_t numericValueSize = 0;
    switch (sample_encoding) {
    case Protections::SampleEncoding::BOOL:
        numericValueSize = 1;
        break;
    case Protections::SampleEncoding::FLOAT32:
        numericValueSize = 4;
        break;
    case Protections::SampleEncoding::FLOAT64:
    case Protections::SampleEncoding::SIGNED:
    case Protections::SampleEncoding::UNSIGNED:
        numericValueSize = 8;
        break;
    default:
        break;
    }
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
bool ProtectionADJv3Packet::tcp_client_send(
    ProtectionADJv3Packet::Package& pack,
    const void* value
) {
    if (tpcb == nullptr || !is_connected) {
        return false;
    }

    memcpy(pack.value, value, pack.packageSize - sizeof(Data));
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
