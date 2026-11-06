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
    uint16_t measurement_id,
    Protections::RuleKind prot,
    Protections::SampleEncoding valueSize
) {
    // Bit packing of protection into 3 most significant bits of id (Look adjv3 format)
    uint8_t numericValueSize;
    switch (valueSize) {
    case Protections::SampleEncoding::BOOL:
        numericValueSize = 1;
        break;
    case Protections::SampleEncoding::FLOAT32:
        numericValueSize = 4;
        break;
    case Protections::SampleEncoding::FLOAT64:
        numericValueSize = 8;
        break;
    case Protections::SampleEncoding::SIGNED:
        numericValueSize = 8;
        break;
    case Protections::SampleEncoding::UNSIGNED:
        numericValueSize = 8;
        break;
    default:
        break;
    }
    Package pack = {.packageSize = sizeof(Data) + numericValueSize, .data = {}};
    pack.data.id = (measurement_id & 0x1FFF) | (static_cast<uint16_t>(prot) << 13);
    pack.data.hours = 0;
    pack.data.minutes = 0;
    pack.data.seconds = 0;
    pack.data.subseconds = 0;
    pack.data.reserved = 0;
    return pack;
}

static err_t tcp_client_connected_callback(void* arg, struct tcp_pcb* pcb, err_t err) {
    LWIP_UNUSED_ARG(arg);
    LWIP_UNUSED_ARG(pcb);
    if (err == ERR_OK) {
        ProtectionADJv3Packet::is_connected = true;
    }
    return ERR_OK;
}
static void tcp_client_err_callback(void* arg, err_t err) {
    LWIP_UNUSED_ARG(arg);
    LWIP_UNUSED_ARG(err);
    ProtectionADJv3Packet::tpcb = nullptr; // lwip already has freed the tpcb, only pointer is left
    ProtectionADJv3Packet::is_connected = false;
}

bool ProtectionADJv3Packet::tcp_client_init(const char* ip, uint16_t port) {
    if (tpcb != nullptr) { // it means we already have one tpcb initialized
        return false;
    }
    ip_addr_t destIPADDR;
    if (!ip4addr_aton(ip, &destIPADDR)) {
        return false; // ip was not converted
    }
    tpcb = tcp_new();
    if (tpcb == nullptr) {
        return false; // tpcb was not initialized
    }
    tcp_err(tpcb, tcp_client_err_callback);
    err_t err = tcp_connect(tpcb, &destIPADDR, port, tcp_client_connected_callback);
    if (err != ERR_OK) { // we got an error while connecting
        tcp_abort(tpcb);
        tpcb = nullptr;
        return false;
    }
    return true;
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
