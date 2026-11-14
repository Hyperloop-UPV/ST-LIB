#pragma once

#include "ST-LIB_HIGH/Protections/ProtectionConcepts.hpp"
#include "ST-LIB_HIGH/Protections/ProtectionErrors.hpp"
#include "ST-LIB_HIGH/Protections/ProtectionTypes.hpp"

namespace Protections {

template <typename T> struct BelowRuleConfig {
    T fault_threshold{};
    optional<T> warning_threshold{};
};

template <typename T> struct AboveRuleConfig {
    T fault_threshold{};
    optional<T> warning_threshold{};
};

template <typename T> struct RangeRuleConfig {
    T low_fault{};
    T high_fault{};
    optional<T> low_warning{};
    optional<T> high_warning{};
};

template <typename T> struct EqualsRuleConfig {
    T expected{};
    bool is_fault = true;
};

template <typename T> struct NotEqualsRuleConfig {
    T expected{};
    bool is_fault = true;
};

template <typename T> struct TimeAccumulationRuleConfig {
    T fault_threshold{};
    optional<T> warning_threshold{};
    float time_window_s{0.0f};
    RuleKind condition_kind{RuleKind::ABOVE};
    bool is_fault{true};
};

template <ProtectionSample T>
using RuleDefinition = variant<
    BelowRuleConfig<T>,
    AboveRuleConfig<T>,
    RangeRuleConfig<T>,
    EqualsRuleConfig<T>,
    NotEqualsRuleConfig<T>,
    TimeAccumulationRuleConfig<T>>;

namespace detail {

template <typename Error, typename Predicate>
constexpr expected<void, Error> validate_runtime(Predicate predicate, Error error) {
    if (!predicate()) {
        return unexpected(error);
    }
    return {};
}

template <typename Error, typename Predicate>
constexpr expected<void, Error>
validate_with_consteval(Predicate predicate, Error error, const char* message) {
    if consteval {
        if (!predicate()) {
            (void)message;
            return unexpected(error);
        }
    }
    return validate_runtime(predicate, error);
}

template <ProtectionSample T>
constexpr expected<RuleDefinition<T>, RuleConfigError>
validate_below(optional<T> warning_threshold, T fault_threshold) {
    if (!warning_threshold.has_value()) {
        return RuleDefinition<T>{BelowRuleConfig<T>{.fault_threshold = fault_threshold}};
    }

    const auto validation = validate_with_consteval<RuleConfigError>(
        [&] { return warning_threshold.value() >= fault_threshold; },
        RuleConfigError::INVALID_WARNING_THRESHOLD,
        "below warning threshold must be above or equal to the fault threshold"
    );
    if (!validation.has_value()) {
        return unexpected(validation.error());
    }

    return RuleDefinition<T>{BelowRuleConfig<T>{
        .fault_threshold = fault_threshold,
        .warning_threshold = warning_threshold,
    }};
}

template <ProtectionSample T>
constexpr expected<RuleDefinition<T>, RuleConfigError>
validate_above(optional<T> warning_threshold, T fault_threshold) {
    if (!warning_threshold.has_value()) {
        return RuleDefinition<T>{AboveRuleConfig<T>{.fault_threshold = fault_threshold}};
    }

    const auto validation = validate_with_consteval<RuleConfigError>(
        [&] { return warning_threshold.value() <= fault_threshold; },
        RuleConfigError::INVALID_WARNING_THRESHOLD,
        "above warning threshold must be below or equal to the fault threshold"
    );
    if (!validation.has_value()) {
        return unexpected(validation.error());
    }

    return RuleDefinition<T>{AboveRuleConfig<T>{
        .fault_threshold = fault_threshold,
        .warning_threshold = warning_threshold,
    }};
}

template <ProtectionSample T>
constexpr expected<RuleDefinition<T>, RuleConfigError>
validate_range(T low_fault, T high_fault, optional<T> low_warning, optional<T> high_warning) {
    const auto fault_validation = validate_with_consteval<RuleConfigError>(
        [&] { return low_fault <= high_fault; },
        RuleConfigError::INVALID_RANGE_THRESHOLDS,
        "range low fault threshold must be below or equal to high fault threshold"
    );
    if (!fault_validation.has_value()) {
        return unexpected(fault_validation.error());
    }

    if (low_warning.has_value() != high_warning.has_value()) {
        return unexpected(RuleConfigError::INVALID_RANGE_THRESHOLDS);
    }

    if (low_warning.has_value()) {
        const auto warning_validation = validate_with_consteval<RuleConfigError>(
            [&] {
                return low_fault <= low_warning.value() &&
                       low_warning.value() <= high_warning.value() &&
                       high_warning.value() <= high_fault;
            },
            RuleConfigError::INVALID_RANGE_THRESHOLDS,
            "range warning thresholds must stay inside the fault range"
        );
        if (!warning_validation.has_value()) {
            return unexpected(warning_validation.error());
        }
    }

    return RuleDefinition<T>{RangeRuleConfig<T>{
        .low_fault = low_fault,
        .high_fault = high_fault,
        .low_warning = low_warning,
        .high_warning = high_warning,
    }};
}

template <FloatingSample T>
constexpr expected<RuleDefinition<T>, RuleConfigError>
validate_time_accumulation(T fault_threshold, optional<T> warning_threshold, float window_seconds) {
    const auto window_validation = validate_with_consteval<RuleConfigError>(
        [&] { return window_seconds > 0.0f; },
        RuleConfigError::INVALID_WINDOW,
        "time_accumulation requires a positive window"
    );
    if (!window_validation.has_value()) {
        return unexpected(window_validation.error());
    }

    if (warning_threshold.has_value()) {
        const auto warning_validation = validate_with_consteval<RuleConfigError>(
            [&] { return warning_threshold.value() <= fault_threshold; },
            RuleConfigError::INVALID_WARNING_THRESHOLD,
            "time_accumulation warning threshold must be below or equal to the fault threshold"
        );
        if (!warning_validation.has_value()) {
            return unexpected(warning_validation.error());
        }
    }

    return RuleDefinition<T>{TimeAccumulationRuleConfig<T>{
        .fault_threshold = fault_threshold,
        .warning_threshold = warning_threshold,
        .time_window_s = window_seconds,
    }};
}

} // namespace detail

namespace Rules {

template <ComparableSample T>
constexpr expected<RuleDefinition<T>, RuleConfigError> below(T fault_threshold) {
    return detail::validate_below<T>(nullopt, fault_threshold);
}

template <ComparableSample T>
constexpr expected<RuleDefinition<T>, RuleConfigError>
below(T fault_threshold, T warning_threshold) {
    return detail::validate_below<T>(warning_threshold, fault_threshold);
}

template <ComparableSample T>
constexpr expected<RuleDefinition<T>, RuleConfigError> below_warning(T warning_threshold) {
    return detail::validate_below<T>(warning_threshold, std::numeric_limits<T>::lowest());
}

template <ComparableSample T>
constexpr expected<RuleDefinition<T>, RuleConfigError> above(T fault_threshold) {
    return detail::validate_above<T>(nullopt, fault_threshold);
}

template <ComparableSample T>
constexpr expected<RuleDefinition<T>, RuleConfigError>
above(T fault_threshold, T warning_threshold) {
    return detail::validate_above<T>(warning_threshold, fault_threshold);
}
template <ComparableSample T>
constexpr expected<RuleDefinition<T>, RuleConfigError> above_warning(T warning_threshold) {
    return detail::validate_above<T>(warning_threshold, std::numeric_limits<T>::max());
}
template <ComparableSample T>
constexpr expected<RuleDefinition<T>, RuleConfigError> range(T low_fault, T high_fault) {
    return detail::validate_range<T>(low_fault, high_fault, nullopt, nullopt);
}

template <ComparableSample T>
constexpr expected<RuleDefinition<T>, RuleConfigError>
range(T low_fault, T high_fault, T low_warning, T high_warning) {
    return detail::validate_range<T>(low_fault, high_fault, low_warning, high_warning);
}
template <ComparableSample T>
constexpr expected<RuleDefinition<T>, RuleConfigError>
range_warning(T low_warning, T high_warning) {
    return detail::validate_range<T>(
        std::numeric_limits<T>::lowest(),
        std::numeric_limits<T>::max(),
        low_warning,
        high_warning
    );
}
template <EqualityComparableSample T>
constexpr expected<RuleDefinition<T>, RuleConfigError> equals(T value) {
    return RuleDefinition<T>{EqualsRuleConfig<T>{.expected = value}};
}

template <EqualityComparableSample T>
constexpr expected<RuleDefinition<T>, RuleConfigError> equals_warning(T value) {
    return RuleDefinition<T>{EqualsRuleConfig<T>{.expected = value, .is_fault = false}};
}

template <EqualityComparableSample T>
constexpr expected<RuleDefinition<T>, RuleConfigError> not_equals(T value) {
    return RuleDefinition<T>{NotEqualsRuleConfig<T>{.expected = value}};
}

template <EqualityComparableSample T>
constexpr expected<RuleDefinition<T>, RuleConfigError> not_equals_warning(T value) {
    return RuleDefinition<T>{NotEqualsRuleConfig<T>{.expected = value, .is_fault = false}};
}

template <FloatingSample T>
constexpr expected<RuleDefinition<T>, RuleConfigError>
time_accumulation(T fault_threshold, float window_seconds) {
    return detail::validate_time_accumulation<T>(fault_threshold, nullopt, window_seconds);
}
// this time_accumulation function is for below rule only
template <FloatingSample T>
constexpr expected<RuleDefinition<T>, RuleConfigError>
time_accumulation_below(T fault_threshold, float window_seconds) {
    return detail::validate_time_accumulation<T>(fault_threshold, nullopt, window_seconds, true);
}
// this time_accumulation function is for other rules left
template <FloatingSample T>
constexpr expected<RuleDefinition<T>, RuleConfigError>
time_accumulation_above(T fault_threshold, float window_seconds) {
    return detail::validate_time_accumulation<T>(fault_threshold, nullopt, window_seconds, false);
}
template <ProtectionSample T>
constexpr expected<RuleDefinition<T>, RuleConfigError> time_accumulation(
    T threshold,
    float window_seconds,
    RuleKind condition = RuleKind::ABOVE,
    bool is_fault = true
) {
    if (window_seconds <= 0.0f) {
        return unexpected(RuleConfigError::INVALID_WINDOW);
    }
    return RuleDefinition<T>{TimeAccumulationRuleConfig<T>{
        .fault_threshold = threshold,
        .warning_threshold = nullopt,
        .time_window_s = window_seconds,
        .condition_kind = condition,
        .is_fault = is_fault,
    }};
}

} // namespace Rules
} // namespace Protections
