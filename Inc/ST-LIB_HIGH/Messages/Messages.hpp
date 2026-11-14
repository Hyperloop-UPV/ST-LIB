#pragma once
/*
 * Messages.hpp
 * "Hello there"
 *  Created on: Nov 13, 2026
 *      Author: Aleks
 */
#include <cstdint>
#include <cstddef>
struct tcp_pcb;
inline const uint8_t max_origin_size = 48;
inline const uint16_t max_message_size = 320;

namespace ST_LIB {
struct MessageADJv3Packet {
    struct Data {
        uint16_t id;
        uint8_t hours;
        uint8_t minutes;
        uint8_t seconds;
        uint8_t subseconds;
    };
    struct Package {
        size_t packageSize;
        Data data;
        uint8_t payload[max_origin_size + max_message_size]; // 48 bytes of origin and 320 of
                                                             // message as maximum length
    };

public:
    static Package make(uint16_t id, const char* origin, const char* message);
    static bool tcp_client_send(Package& pack);
    static void create_message(uint16_t id, const char* origin, const char* message);

    static struct tcp_pcb* tpcb;
    static bool is_connected;
};
} // namespace ST_LIB
