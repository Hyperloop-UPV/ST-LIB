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

namespace ST_LIB {
struct ProtectionPacket {

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
        uint16_t protection_id,
        Protections::RuleKind protection,
        Protections::SampleEncoding sample_encoding
    );
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
                    return Protections::protection<protection_id, measurement>(
                        Protections::Rules::range(firstCriticalValue, secondCriticalValue)
                    );
                }
                if (kind == Protections::RuleKind::EQUALS) {
                    return Protections::protection<protection_id, measurement>(
                        Protections::Rules::equals(firstCriticalValue)
                    );
                }
                if (kind == Protections::RuleKind::ABOVE) {
                    return Protections::protection<protection_id, measurement>(
                        Protections::Rules::above(firstCriticalValue)
                    );
                }
                if (kind == Protections::RuleKind::BELOW) {
                    return Protections::protection<protection_id, measurement>(
                        Protections::Rules::below(firstCriticalValue)
                    );
                }

                if (kind == Protections::RuleKind::NOT_EQUALS) {
                    return Protections::protection<protection_id, measurement>(
                        Protections::Rules::not_equals(firstCriticalValue)
                    );
                }
            } else { // doWarning
                if (kind == Protections::RuleKind::RANGE) {
                    return Protections::protection<protection_id, measurement>(
                        Protections::Rules::range_warning(firstCriticalValue, secondCriticalValue)
                    );
                }
                if (kind == Protections::RuleKind::EQUALS) {
                    return Protections::protection<protection_id, measurement>(
                        Protections::Rules::equals_warning(firstCriticalValue)
                    );
                }
                if (kind == Protections::RuleKind::ABOVE) {
                    return Protections::protection<protection_id, measurement>(
                        Protections::Rules::above_warning(firstCriticalValue)
                    );
                }
                if (kind == Protections::RuleKind::BELOW) {
                    return Protections::protection<protection_id, measurement>(
                        Protections::Rules::below_warning(firstCriticalValue)
                    );
                }

                if (kind == Protections::RuleKind::NOT_EQUALS) {
                    return Protections::protection<protection_id, measurement>(
                        Protections::Rules::not_equals_warning(firstCriticalValue)
                    );
                }
            }
        } else { // protections which will be sent if value is out of range during some time
            float time_s = static_cast<float>(cooldown_micros) / 1'000'000.0f;
            return Protections::protection<protection_id, measurement>(
                Protections::Rules::time_accumulation(firstCriticalValue, time_s, kind, doFault)
            );
        }
        return Protections::protection<protection_id, measurement>(
            Protections::Rules::equals(firstCriticalValue)
        );
    }
};

} // namespace ST_LIB
