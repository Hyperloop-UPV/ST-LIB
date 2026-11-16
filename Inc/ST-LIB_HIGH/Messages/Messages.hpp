#pragma once
/*
 * Messages.hpp
 * "Hello there"
 *  Created on: Nov 13, 2026
 *      Author: Aleks
 */
#include <cstdint>
#include <cstddef>
#include <type_traits>

inline const uint8_t max_origin_size = 48;
inline const uint16_t max_message_size = 320;

namespace ST_LIB {

union MessageKindValues_t {
    struct {
        uint8_t INFO;
        uint8_t WARNING;
        uint8_t FAULT;
        uint8_t PANIC;
    } val;
    uint8_t arr[4];
};

inline MessageKindValues_t MessageKindValues = {
    .val = {.INFO = 12, .WARNING = 11, .FAULT = 10, .PANIC = 13}
};

enum class MessageKind : uint8_t { INFO = 0, WARNING = 1, FAULT = 2, PANIC = 3 };

struct MessagePacket {
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
    static bool create_message(uint16_t id, const char* origin, const char* message);
    static bool create_message(MessageKind kind, const char* origin, const char* message) {
        return create_message(MessageKindValues.arr[static_cast<size_t>(kind)], origin, message);
    }
};

inline bool create_message(uint16_t id, const char* origin, const char* message) {
    return MessagePacket::create_message(id, origin, message);
}

inline bool create_message(MessageKind kind, const char* origin, const char* message) {
    return MessagePacket::create_message(kind, origin, message);
}

} // namespace ST_LIB
