#pragma once
/*
 * ProtectionMessages.hpp
 *      "Hello there"
 *  Created on: Oct 22, 2026
 *      Author: Aleks
 */
#include "ST-LIB_HIGH/Protections/ProtectionTypes.hpp"
#include <cstdint>
#include <cstddef>
#include <array>
#include <initializer_list>
struct tcp_pcb;

namespace ST_LIB {
struct ProtectionCodeTable { // global structure that is used to define order of protections for
                             // every id we have (and will be used to generate code later)
    struct Entry {
        uint16_t measurement_id{0};
        std::array<Protections::RuleKind, 7> order{};
        uint8_t count{0};
    };

    static std::array<Entry, 32> entries;
    static inline uint8_t entries_count{0};

    static uint8_t find(uint16_t id, Protections::RuleKind kind) {
        for (size_t j = 0; j < entries_count; ++j) {
            if (entries[j].measurement_id != id) {
                continue;
            }
            for (size_t i = 0; i < entries[j].count; ++i) {
                if (entries[j].order[i] == kind) {
                    return static_cast<uint8_t>(i + 1);
                }
            }
            break;
        }
        return 1;
    }

    static void set(uint16_t id, std::initializer_list<Protections::RuleKind> kinds) {
        Entry* target = nullptr;
        for (size_t j = 0; j < entries_count; ++j) {
            if (entries[j].measurement_id == id) {
                target = &entries[j];
                break;
            }
        }
        if (target == nullptr) {
            if (entries_count >= entries.size()) {
                return;
            }
            target = &entries[entries_count++];
            target->measurement_id = id;
        }
        target->count = 0;
        for (Protections::RuleKind kind : kinds) {
            if (target->count >= target->order.size()) {
                break;
            }
            target->order[target->count++] = kind;
        }
    }
};

inline std::array<ProtectionCodeTable::Entry, 32> ProtectionCodeTable::entries{};
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
    static Package make(
        uint16_t measurement_id,
        Protections::RuleKind protection,
        Protections::SampleEncoding value_size
    );
    static bool tcp_client_send(Package& pack, const void* value);

    static struct tcp_pcb* tpcb;
    static bool is_connected;
};

} // namespace ST_LIB
