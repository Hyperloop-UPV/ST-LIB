#pragma once
/*
 * ProtectionMessages.hpp
 *      "Hello there"
 *  Created on: Oct 22, 2026
 *      Author: Aleks
 */
#include "ST-LIB_HIGH/Protections/ProtectionTypes.hpp"
#include "ST-LIB_HIGH/Protections/ProtectionEngine.hpp"
#include "ST-LIB_HIGH/Protections/Rules.hpp"
#include <cstdint>
#include <cstddef>
#include <array>
#include <initializer_list>
struct tcp_pcb;

namespace ST_LIB {
// struct ProtectionCodeTable { // global structure that is used to define order of protections for
//                              // every id we have (and will be used to generate code later)
//     struct Entry {
//         uint16_t measurement_id{0};
//         std::array<Protections::RuleKind, 7> order{};
//         uint8_t count{0};
//     };

//     static std::array<Entry, 32> entries;
//     static inline uint8_t entries_count{0};

//     static uint8_t find(uint16_t id, Protections::RuleKind kind) {
//         for (size_t j = 0; j < entries_count; ++j) {
//             if (entries[j].measurement_id != id) {
//                 continue;
//             }
//             for (size_t i = 0; i < entries[j].count; ++i) {
//                 if (entries[j].order[i] == kind) {
//                     return static_cast<uint8_t>(i + 1);
//                 }
//             }
//             break;
//         }
//         return 1;
//     }

//     static void set(uint16_t id, std::initializer_list<Protections::RuleKind> kinds) {
//         Entry* target = nullptr;
//         for (size_t j = 0; j < entries_count; ++j) {
//             if (entries[j].measurement_id == id) {
//                 target = &entries[j];
//                 break;
//             }
//         }
//         if (target == nullptr) {
//             if (entries_count >= entries.size()) {
//                 return;
//             }
//             target = &entries[entries_count++];
//             target->measurement_id = id;
//         }
//         target->count = 0;
//         for (Protections::RuleKind kind : kinds) {
//             if (target->count >= target->order.size()) {
//                 break;
//             }
//             target->order[target->count++] = kind;
//         }
//     }
// };

// inline std::array<ProtectionCodeTable::Entry, 32> ProtectionCodeTable::entries{};
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
    make(uint16_t protection_id, Protections::RuleKind protection, Protections::SampleEncoding sample_encoding);
    static bool tcp_client_send(Package& pack, const void* value);
    template <uint16_t protection_id, auto& measurement, typename T>
    static constexpr auto create_protection(
        Protections::RuleKind kind,
        bool doFault,
        uint64_t cooldown_micros,
        T firstCriticalValue,
        T secondCriticalValue = {}
    ) {
        constexpr uint16_t measurement_id = protection_id & 0x1FFF;
        constexpr uint8_t position = (protection_id >> 13) & 0x07;
        (void)measurement_id;
        (void)position;

        if (cooldown_micros ==
            0) { // protections which will be sent instanly when value is out of range
            if (doFault) {
                if (kind == Protections::RuleKind::RANGE) {
                    return Protections::protection<"", protection_id, measurement>(
                        Protections::Rules::range(firstCriticalValue, secondCriticalValue)
                    );
                }
                if (kind == Protections::RuleKind::EQUALS) {
                    return Protections::protection<"", protection_id, measurement>(
                        Protections::Rules::equals(firstCriticalValue)
                    );
                }
                if (kind == Protections::RuleKind::ABOVE) {
                    return Protections::protection<"", protection_id, measurement>(
                        Protections::Rules::above(firstCriticalValue)
                    );
                }
                if (kind == Protections::RuleKind::BELOW) {
                    return Protections::protection<"", protection_id, measurement>(
                        Protections::Rules::below(firstCriticalValue)
                    );
                }

                if (kind == Protections::RuleKind::NOT_EQUALS) {
                    return Protections::protection<"", protection_id, measurement>(
                        Protections::Rules::not_equals(firstCriticalValue)
                    );
                }
            } else { // doWarning
                if (kind == Protections::RuleKind::RANGE) {
                    return Protections::protection<"", protection_id, measurement>(
                        Protections::Rules::range_warning(firstCriticalValue, secondCriticalValue)
                    );
                }
                if (kind == Protections::RuleKind::EQUALS) {
                    return Protections::protection<"", protection_id, measurement>(
                        Protections::Rules::equals_warning(firstCriticalValue)
                    );
                }
                if (kind == Protections::RuleKind::ABOVE) {
                    return Protections::protection<"", protection_id, measurement>(
                        Protections::Rules::above_warning(firstCriticalValue)
                    );
                }
                if (kind == Protections::RuleKind::BELOW) {
                    return Protections::protection<"", protection_id, measurement>(
                        Protections::Rules::below_warning(firstCriticalValue)
                    );
                }

                if (kind == Protections::RuleKind::NOT_EQUALS) {
                    return Protections::protection<"", protection_id, measurement>(
                        Protections::Rules::not_equals_warning(firstCriticalValue)
                    );
                }
            }
        } else { // protections which will be sent if value is out of range during some time
            float time_s = static_cast<float>(cooldown_micros) / 1'000'000.0f;
            return Protections::protection<"", protection_id, measurement>(
                Protections::Rules::time_accumulation(firstCriticalValue, time_s, kind, doFault)
            );
        }
        return Protections::protection<"", protection_id, measurement>(
            Protections::Rules::equals(firstCriticalValue)
        );
    }
    static struct tcp_pcb* tpcb;
    static bool is_connected;
};

}; // namespace ST_LIB
