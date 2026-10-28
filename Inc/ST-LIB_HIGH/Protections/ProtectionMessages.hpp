#pragma once
/*
 * ProtectionMessages.hpp
 *      "Hello there"
 *  Created on: Oct 22, 2026
 *      Author: Aleks
 */
#include "ST-LIB_HIGH/Protections/SampleSource.hpp"
#include "HALAL/Services/Diagnostics/Diagnostics.hpp"
#include "ST-LIB_HIGH/Protections/FaultController.hpp"
#include "ST-LIB_HIGH/Protections/Protection.hpp"
#include "ST-LIB_HIGH/Protections/ProtectionTypes.hpp"
#include <cstdint>
#include <cstddef>
#include <cstring>

struct tcp_pcb;

namespace ST_LIB {
struct ProtectionADJv3Packet {

    struct Data {
        uint16_t id;
        uint8_t hours;
        uint8_t minutes;
        uint8_t seconds;
        uint8_t subseconds;
        uint16_t reserved; // Just to not forget about it exists
    };

    struct Package {
        size_t packageSize;
        Data data;
        // Maximum is 8 bytes, not all will be sent always
        uint8_t value[8];
    };

public:
    static Package
    make(uint16_t measurement_id, Protections::RuleKind protection, size_t value_size);
    static bool tcp_client_init(const char* ip, uint16_t port);
    static bool tcp_client_send(Package& pack, const void* value);

    static struct tcp_pcb* tpcb;
    static bool is_connected;
};

} // namespace ST_LIB
