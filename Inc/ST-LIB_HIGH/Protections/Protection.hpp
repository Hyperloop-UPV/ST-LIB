#pragma once

#include <cmath>

#include "C++Utilities/CppUtils.hpp"
#include "HALAL/Services/Time/Scheduler.hpp"
#include "ST-LIB_HIGH/Protections/ProtectionTypes.hpp"
#include "ST-LIB_HIGH/Protections/Rules.hpp"
#include "ST-LIB_HIGH/Protections/SampleSource.hpp"

namespace Protections {

namespace detail {

template <typename T> constexpr T zero_value() { return static_cast<T>(0); }

template <typename T> constexpr T absolute_value(T value) {
    if constexpr (std::is_floating_point_v<T>) {
        return static_cast<T>(std::fabs(value));
    } else if constexpr (std::is_signed_v<T>) {
        return value < 0 ? static_cast<T>(-value) : value;
    } else {
        return value;
    }
}

template <typename T> constexpr bool is_below(T sample, T threshold) { return sample < threshold; }

template <typename T> constexpr bool is_above(T sample, T threshold) { return sample > threshold; }

template <typename T> constexpr bool is_equal_to(T lhs, T rhs) { return lhs == rhs; }

template <typename T> constexpr bool is_not_equal_to(T lhs, T rhs) {
    return !is_equal_to(lhs, rhs);
}

} // namespace detail

class RuleStateTracker {
public:
    constexpr RuleState previous_state() const { return last_state; }

    constexpr RuleEdge advance(RuleState current_state) {
        RuleEdge edge = RuleEdge::NONE;
        if (current_state == RuleState::FAULT && last_state != RuleState::FAULT) {
            edge = RuleEdge::FAULT_RAISED;
        } else if (current_state == RuleState::WARNING && last_state != RuleState::WARNING) {
            edge = RuleEdge::WARNING_RAISED;
        } else if (current_state == RuleState::NORMAL && last_state != RuleState::NORMAL) {
            edge = RuleEdge::RECOVERED;
        }

        last_state = current_state;
        return edge;
    }

private:
    RuleState last_state{RuleState::NORMAL};
};

class RuleSnapshotBuilder {
public:
    template <ProtectionSample T>
    static RuleSnapshot single_threshold(
        RuleKind kind,
        T observed,
        RuleState current_state,
        RuleEdge edge,
        RuleState previous_state,
        T fault_threshold,
        optional<T> warning_threshold,
        float time_window_s = 0.0f,
        float active_time_s = 0.0f
    ) {
        RuleSnapshot snapshot{};
        snapshot.kind = kind;
        snapshot.sample_encoding = sample_encoding_for<T>();
        snapshot.observed_value = to_numeric_value(observed);
        snapshot.time_window_s = time_window_s;
        snapshot.active_time_s = active_time_s;
        snapshot.uses_warning_threshold = should_use_warning_threshold(
            current_state,
            edge,
            previous_state,
            warning_threshold.has_value()
        );

        snapshot.threshold_a = to_numeric_value(
            snapshot.uses_warning_threshold ? warning_threshold.value_or(fault_threshold)
                                            : fault_threshold
        );
        return snapshot;
    }

    template <ProtectionSample T>
    static RuleSnapshot range(
        T observed,
        RuleState current_state,
        RuleEdge edge,
        RuleState previous_state,
        const RangeRuleConfig<T>& config
    ) {
        RuleSnapshot snapshot{};
        snapshot.kind = RuleKind::RANGE;
        snapshot.sample_encoding = sample_encoding_for<T>();
        snapshot.observed_value = to_numeric_value(observed);
        snapshot.has_threshold_b = true;
        snapshot.uses_warning_threshold = should_use_warning_threshold(
            current_state,
            edge,
            previous_state,
            config.low_warning.has_value() && config.high_warning.has_value()
        );
        snapshot.threshold_a = to_numeric_value(
            snapshot.uses_warning_threshold ? config.low_warning.value_or(config.low_fault)
                                            : config.low_fault
        );
        snapshot.threshold_b = to_numeric_value(
            snapshot.uses_warning_threshold ? config.high_warning.value_or(config.high_fault)
                                            : config.high_fault
        );
        return snapshot;
    }

private:
    static constexpr bool should_use_warning_threshold(
        RuleState current_state,
        RuleEdge edge,
        RuleState previous_state,
        bool has_warning_threshold
    ) {
        return has_warning_threshold &&
               (current_state == RuleState::WARNING ||
                (edge == RuleEdge::RECOVERED && previous_state == RuleState::WARNING));
    }
};

template <ComparableSample T> struct BelowEvaluator {
    static constexpr RuleState compute(const T& sample, const BelowRuleConfig<T>& config) {
        if (detail::is_below(sample, config.fault_threshold)) {
            return RuleState::FAULT;
        }
        if (config.warning_threshold.has_value() &&
            detail::is_below(sample, config.warning_threshold.value())) {
            return RuleState::WARNING;
        }
        return RuleState::NORMAL;
    }
};

template <ComparableSample T> struct AboveEvaluator {
    static constexpr RuleState compute(const T& sample, const AboveRuleConfig<T>& config) {
        if (detail::is_above(sample, config.fault_threshold)) {
            return RuleState::FAULT;
        }
        if (config.warning_threshold.has_value() &&
            detail::is_above(sample, config.warning_threshold.value())) {
            return RuleState::WARNING;
        }
        return RuleState::NORMAL;
    }
};

template <ComparableSample T> struct RangeEvaluator {
    static constexpr RuleState compute(const T& sample, const RangeRuleConfig<T>& config) {
        if (detail::is_below(sample, config.low_fault) ||
            detail::is_above(sample, config.high_fault)) {
            return RuleState::FAULT;
        }
        if (config.low_warning.has_value() && config.high_warning.has_value() &&
            (detail::is_below(sample, config.low_warning.value()) ||
             detail::is_above(sample, config.high_warning.value()))) {
            return RuleState::WARNING;
        }
        return RuleState::NORMAL;
    }
};

template <EqualityComparableSample T> struct EqualsEvaluator {
    static constexpr RuleState compute(const T& sample, const EqualsRuleConfig<T>& config) {
        if (detail::is_equal_to(sample, config.expected)) {
            return config.is_fault ? RuleState::FAULT : RuleState::WARNING;
        }
        return RuleState::NORMAL;
    }
};

template <EqualityComparableSample T> struct NotEqualsEvaluator {
    static constexpr RuleState compute(const T& sample, const NotEqualsRuleConfig<T>& config) {
        if (detail::is_not_equal_to(sample, config.expected)) {
            return config.is_fault ? RuleState::FAULT : RuleState::WARNING;
        }
        return RuleState::NORMAL;
    }
};

template <ProtectionSample T> struct TimeAccumulationEvaluator {
    static RuleState compute(
        const T& sample,
        const TimeAccumulationRuleConfig<T>& config,
        uint64_t configured_window_us,
        bool& has_last_tick,
        uint64_t& last_tick_us,
        uint64_t& warning_active_time_us,
        uint64_t& fault_active_time_us,
        T& active_magnitude,
        float& active_time_s
    ) {
        const uint64_t now_us = Scheduler::get_global_tick();
        const uint64_t elapsed_us = has_last_tick ? (now_us - last_tick_us) : 0ULL;
        has_last_tick = true;
        last_tick_us = now_us;

        active_magnitude = detail::absolute_value(sample);

        bool fault_condition = false;
        switch (config.condition_kind) {
        case RuleKind::EQUALS:
            fault_condition = detail::is_equal_to(sample, config.fault_threshold);
            break;
        case RuleKind::NOT_EQUALS:
            fault_condition = detail::is_not_equal_to(sample, config.fault_threshold);
            break;
        case RuleKind::BELOW:
            fault_condition = detail::is_below(sample, config.fault_threshold);
            break;
        case RuleKind::ABOVE:
        default:
            fault_condition = detail::is_above(sample, config.fault_threshold);
            break;
        }
        if (fault_condition) {
            fault_active_time_us += elapsed_us;
        } else {
            fault_active_time_us = 0;
        }
        if (config.warning_threshold.has_value() &&
            detail::is_above(active_magnitude, config.warning_threshold.value())) {
            warning_active_time_us += elapsed_us;
        } else {
            warning_active_time_us = 0;
        }

        if (fault_active_time_us >= configured_window_us) {
            active_time_s = static_cast<float>(fault_active_time_us) / 1'000'000.0f;
            return config.is_fault ? RuleState::FAULT : RuleState::WARNING;
        }

        active_time_s =
            static_cast<float>(
                config.warning_threshold.has_value() ? warning_active_time_us : fault_active_time_us
            ) /
            1'000'000.0f;
        return RuleState::NORMAL;
    }
};

template <ComparableSample T> struct BelowRule {
    BelowRuleConfig<T> config{};
    RuleStateTracker tracker{};

    RuleEvaluation evaluate(const T& sample) {
        const RuleState previous_state = tracker.previous_state();
        const RuleState state = BelowEvaluator<T>::compute(sample, config);
        const RuleEdge edge = tracker.advance(state);
        return {
            .state = state,
            .edge = edge,
            .snapshot = RuleSnapshotBuilder::single_threshold(
                RuleKind::BELOW,
                sample,
                state,
                edge,
                previous_state,
                config.fault_threshold,
                config.warning_threshold
            ),
        };
    }
};

template <ComparableSample T> struct AboveRule {
    AboveRuleConfig<T> config{};
    RuleStateTracker tracker{};

    RuleEvaluation evaluate(const T& sample) {
        const RuleState previous_state = tracker.previous_state();
        const RuleState state = AboveEvaluator<T>::compute(sample, config);
        const RuleEdge edge = tracker.advance(state);
        return {
            .state = state,
            .edge = edge,
            .snapshot = RuleSnapshotBuilder::single_threshold(
                RuleKind::ABOVE,
                sample,
                state,
                edge,
                previous_state,
                config.fault_threshold,
                config.warning_threshold
            ),
        };
    }
};

template <ComparableSample T> struct RangeRule {
    RangeRuleConfig<T> config{};
    RuleStateTracker tracker{};

    RuleEvaluation evaluate(const T& sample) {
        const RuleState previous_state = tracker.previous_state();
        const RuleState state = RangeEvaluator<T>::compute(sample, config);
        const RuleEdge edge = tracker.advance(state);
        return {
            .state = state,
            .edge = edge,
            .snapshot = RuleSnapshotBuilder::range(sample, state, edge, previous_state, config),
        };
    }
};

template <EqualityComparableSample T> struct EqualsRule {
    EqualsRuleConfig<T> config{};
    RuleStateTracker tracker{};

    RuleEvaluation evaluate(const T& sample) {
        const RuleState previous_state = tracker.previous_state();
        const RuleState state = EqualsEvaluator<T>::compute(sample, config);
        const RuleEdge edge = tracker.advance(state);
        return {
            .state = state,
            .edge = edge,
            .snapshot = RuleSnapshotBuilder::single_threshold(
                RuleKind::EQUALS,
                sample,
                state,
                edge,
                previous_state,
                config.expected,
                optional<T>{}
            ),
        };
    }
};

template <EqualityComparableSample T> struct NotEqualsRule {
    NotEqualsRuleConfig<T> config{};
    RuleStateTracker tracker{};

    RuleEvaluation evaluate(const T& sample) {
        const RuleState previous_state = tracker.previous_state();
        const RuleState state = NotEqualsEvaluator<T>::compute(sample, config);
        const RuleEdge edge = tracker.advance(state);
        return {
            .state = state,
            .edge = edge,
            .snapshot = RuleSnapshotBuilder::single_threshold(
                RuleKind::NOT_EQUALS,
                sample,
                state,
                edge,
                previous_state,
                config.expected,
                optional<T>{}
            ),
        };
    }
};

template <ProtectionSample T> struct TimeAccumulationRule {
    TimeAccumulationRule() = default;

    explicit TimeAccumulationRule(TimeAccumulationRuleConfig<T> config) : config(config) {
        configured_window_us = static_cast<uint64_t>(
            std::llround(static_cast<double>(config.time_window_s) * 1'000'000.0)
        );
        if (configured_window_us == 0) {
            configured_window_us = 1;
        }
    }

    RuleEvaluation evaluate(const T& sample) {
        const RuleState previous_state = tracker.previous_state();
        const RuleState state = TimeAccumulationEvaluator<T>::compute(
            sample,
            config,
            configured_window_us,
            has_last_tick,
            last_tick_us,
            warning_active_time_us,
            fault_active_time_us,
            active_magnitude,
            active_time_s
        );
        const RuleEdge edge = tracker.advance(state);
        return {
            .state = state,
            .edge = edge,
            .snapshot = RuleSnapshotBuilder::single_threshold(
                RuleKind::TIME_ACCUMULATION,
                active_magnitude,
                state,
                edge,
                previous_state,
                config.fault_threshold,
                config.warning_threshold,
                config.time_window_s,
                active_time_s
            ),
        };
    }

    TimeAccumulationRuleConfig<T> config{};
    RuleStateTracker tracker{};
    uint64_t configured_window_us{1};
    bool has_last_tick{false};
    uint64_t last_tick_us{0};
    uint64_t warning_active_time_us{0};
    uint64_t fault_active_time_us{0};
    T active_magnitude{detail::zero_value<T>()};
    float active_time_s{0.0f};
};

template <ProtectionSample T, bool IsFloating = FloatingSample<T>>
struct TimeAccumulationRuleSelector {
    using type = std::monostate;
};

template <ProtectionSample T> struct TimeAccumulationRuleSelector<T, true> {
    using type = TimeAccumulationRule<T>;
};

template <ProtectionSample T>
using TimeAccumulationRuleModel = typename TimeAccumulationRuleSelector<T>::type;

template <ProtectionSample T> struct RuleModel {
    RuleKind kind{RuleKind::BELOW};
    BelowRule<T> below{};
    AboveRule<T> above{};
    RangeRule<T> range{};
    EqualsRule<T> equals{};
    NotEqualsRule<T> not_equals{};
    TimeAccumulationRule<T> time_accumulation{};

    RuleEvaluation evaluate(const T& sample) {
        switch (kind) {
        case RuleKind::BELOW:
            return below.evaluate(sample);
        case RuleKind::ABOVE:
            return above.evaluate(sample);
        case RuleKind::RANGE:
            return range.evaluate(sample);
        case RuleKind::EQUALS:
            return equals.evaluate(sample);
        case RuleKind::NOT_EQUALS:
            return not_equals.evaluate(sample);
        case RuleKind::TIME_ACCUMULATION:
            return time_accumulation.evaluate(sample);
        }
        std::unreachable();
    }
};

template <ProtectionSample T>
inline RuleModel<T> make_rule_model(const RuleDefinition<T>& definition) {
    RuleModel<T> model{};
    switch (definition.index()) {
    case 0:
        model.kind = RuleKind::BELOW;
        model.below.config = std::get<BelowRuleConfig<T>>(definition);
        return model;
    case 1:
        model.kind = RuleKind::ABOVE;
        model.above.config = std::get<AboveRuleConfig<T>>(definition);
        return model;
    case 2:
        model.kind = RuleKind::RANGE;
        model.range.config = std::get<RangeRuleConfig<T>>(definition);
        return model;
    case 3:
        model.kind = RuleKind::EQUALS;
        model.equals.config = std::get<EqualsRuleConfig<T>>(definition);
        return model;
    case 4:
        model.kind = RuleKind::NOT_EQUALS;
        model.not_equals.config = std::get<NotEqualsRuleConfig<T>>(definition);
        return model;
    case 5:
        model.kind = RuleKind::TIME_ACCUMULATION;
        model.time_accumulation =
            TimeAccumulationRule<T>{std::get<TimeAccumulationRuleConfig<T>>(definition)};
        return model;
    default:
        std::unreachable();
    }
}

template <ProtectionSample T, std::size_t RuleCount, ReadableSampleSource Source = SampleSource<T>>
class Protection {
public:
    static_assert(
        std::same_as<std::remove_cvref_t<typename Source::value_type>, T>,
        "Protection source value_type must match the protection sample type"
    );

    Protection(
        const char* name,
        uint16_t measurement_id,
        Source source,
        const std::array<RuleDefinition<T>, RuleCount>& definitions
    )
        : name(name), measurement_id(measurement_id), source(source), definitions(definitions),
          rules(make_rule_models(definitions, std::make_index_sequence<RuleCount>{})) {}

    const char* get_name() const { return name; }
    uint16_t get_measurement_id() const { return measurement_id; }
    void initialize() {}

    ProtectionEvaluation evaluate() {
        ProtectionEvaluation evaluation{};
        const T sample = source.read();

        for (std::size_t index = 0; index < RuleCount; ++index) {
            const RuleEvaluation rule_evaluation = rules[index].evaluate(sample);
            if (rule_evaluation.edge != RuleEdge::NONE &&
                evaluation.event_count < evaluation.events.size()) {
                evaluation.events[evaluation.event_count++] = {
                    rule_evaluation.state,
                    rule_evaluation.edge,
                    rule_evaluation.snapshot,
                };
            }

            if (rule_evaluation.state == RuleState::FAULT && !evaluation.has_active_fault) {
                evaluation.has_active_fault = true;
                evaluation.active_fault_edge = rule_evaluation.edge;
                evaluation.active_fault_snapshot = rule_evaluation.snapshot;
                evaluation.aggregated_state = RuleState::FAULT;
                continue;
            }

            if (evaluation.aggregated_state != RuleState::FAULT &&
                rule_evaluation.state == RuleState::WARNING) {
                evaluation.aggregated_state = RuleState::WARNING;
            }
        }

        return evaluation;
    }

    void clear_runtime_state() {
        rules = make_rule_models(definitions, std::make_index_sequence<RuleCount>{});
        last_fault_publish_tick = 0;
    }

    uint64_t get_last_fault_publish_tick() const { return last_fault_publish_tick; }

    void set_last_fault_publish_tick(uint64_t tick) { last_fault_publish_tick = tick; }

private:
    template <std::size_t... Indices>
    static std::array<RuleModel<T>, RuleCount>
    make_rule_models(const std::array<RuleDefinition<T>, RuleCount>& definitions, std::index_sequence<Indices...>) {
        return {make_rule_model(definitions[Indices])...};
    }

    const char* name{nullptr};
    uint16_t measurement_id;
    Source source;
    std::array<RuleDefinition<T>, RuleCount> definitions{};
    std::array<RuleModel<T>, RuleCount> rules{};
    uint64_t last_fault_publish_tick{0};
};

} // namespace Protections
