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

    template <typename Enum>
        requires std::is_enum_v<Enum>
    static bool create_message(Enum type, const char* origin, const char* message) {
        return create_message(static_cast<uint16_t>(type), origin, message);
    }
};

inline bool create_message(uint16_t id, const char* origin, const char* message) {
    return MessagePacket::create_message(id, origin, message);
}

template <typename Enum>
    requires std::is_enum_v<Enum>
inline bool create_message(Enum type, const char* origin, const char* message) {
    return MessagePacket::create_message(static_cast<uint16_t>(type), origin, message);
}

} // namespace ST_LIB
