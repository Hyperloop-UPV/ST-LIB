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

enum class MessageType : uint16_t { INFO = 1, FAULT = 2, WARNING = 3, PANIC = 13 };

struct MessageADJv3Packet {
    struct __attribute__((packed)) Data {
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
    static bool create_message(MessageType type, const char* origin, const char* message);

    static struct tcp_pcb* tpcb;
    static bool is_connected;
};

inline bool create_message(MessageType type, const char* origin, const char* message) {
    return MessageADJv3Packet::create_message(type, origin, message);
}

} // namespace ST_LIB
