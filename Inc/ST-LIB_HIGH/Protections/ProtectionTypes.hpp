#pragma once

#include "C++Utilities/CppUtils.hpp"
#include "ST-LIB_HIGH/Protections/ProtectionConcepts.hpp"

namespace Protections {

namespace Config {
inline constexpr size_t max_protections = 32;
inline constexpr size_t max_rules_per_protection = 16;
inline constexpr uint64_t notify_delay_in_microseconds = 2'000'000ULL;
inline constexpr size_t max_name_length = 48;
} // namespace Config

enum class RuleKind : uint8_t { RANGE = 1, ABOVE, BELOW, EQUALS, NOT_EQUALS, TIME_ACCUMULATION };

enum class RuleState : uint8_t { NORMAL = 0, WARNING, FAULT };

enum class RuleEdge : uint8_t { NONE = 0, WARNING_RAISED, FAULT_RAISED, RECOVERED };

enum class SampleEncoding : uint8_t { BOOL = 0, SIGNED, UNSIGNED, FLOAT32, FLOAT64 };

struct NumericValue {
    union {
        bool bool_value;
        int64_t signed_value;
        uint64_t unsigned_value;
        float float32_value;
        double float64_value;
    };

    constexpr NumericValue() : unsigned_value(0) {}
};

struct RuleSnapshot {
    RuleKind kind{RuleKind::BELOW};
    SampleEncoding sample_encoding{SampleEncoding::SIGNED};
    NumericValue observed_value{};
    NumericValue threshold_a{};
    NumericValue threshold_b{};
    bool has_threshold_b{false};
    bool uses_warning_threshold{false};
    float time_window_s{0.0f};
    float active_time_s{0.0f};
};

struct RuleEvaluation {
    RuleState state{RuleState::NORMAL};
    RuleEdge edge{RuleEdge::NONE};
    RuleSnapshot snapshot{};
};

struct ProtectionEvent {
    RuleState state{RuleState::NORMAL};
    RuleEdge edge{RuleEdge::NONE};
    RuleSnapshot snapshot{};
};

struct ProtectionEvaluation {
    RuleState aggregated_state{RuleState::NORMAL};
    bool has_active_fault{false};
    RuleEdge active_fault_edge{RuleEdge::NONE};
    RuleSnapshot active_fault_snapshot{};
    array<ProtectionEvent, Config::max_rules_per_protection> events{};
    size_t event_count{0};
};

template <ProtectionSample T> constexpr SampleEncoding sample_encoding_for() {
    if constexpr (std::is_same_v<T, bool>) {
        return SampleEncoding::BOOL;
    } else if constexpr (std::is_floating_point_v<T>) {
        if constexpr (std::is_same_v<T, float>) {
            return SampleEncoding::FLOAT32;
        } else {
            return SampleEncoding::FLOAT64;
        }
    } else if constexpr (std::is_integral_v<T> && std::is_signed_v<T>) {
        return SampleEncoding::SIGNED;
    }
    if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T>) {
        return SampleEncoding::UNSIGNED;
    }
    std::unreachable();
}

template <ProtectionSample T> constexpr NumericValue to_numeric_value(T value) {
    NumericValue numeric{};
    if constexpr (std::is_same_v<T, bool>) {
        numeric.bool_value = value;
    } else if constexpr (std::is_floating_point_v<T>) {
        if constexpr (std::is_same_v<T, float>) {
            numeric.float32_value = value;
        } else {
            numeric.float64_value = value;
        }
    } else if constexpr (std::is_integral_v<T> && std::is_signed_v<T>) {
        numeric.signed_value = static_cast<int64_t>(value);
    } else if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T>) {
        numeric.unsigned_value = static_cast<uint64_t>(value);
    } else {
        std::unreachable();
    }
    return numeric;
}

inline constexpr bool is_fault_state(RuleState state) { return state == RuleState::FAULT; }

inline constexpr bool is_warning_state(RuleState state) { return state == RuleState::WARNING; }

} // namespace Protections
