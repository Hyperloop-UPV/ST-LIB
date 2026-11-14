#pragma once

#include "HALAL/Services/Diagnostics/Diagnostics.hpp"
#if !defined(SIM_ON)
#include "HALAL/Services/Time/RTC.hpp"
#endif
#include "ST-LIB_HIGH/Protections/FaultController.hpp"
#include "ST-LIB_HIGH/Protections/Protection.hpp"

namespace Protections {

namespace detail {

template <typename Source>
concept SampleSourceLike = ReadableSampleSource<Source>;

template <typename Source> consteval auto sample_type_tag() {
    using source_type = std::remove_cvref_t<Source>;
    if constexpr (requires { typename source_type::value_type; }) {
        return std::type_identity<std::remove_cvref_t<typename source_type::value_type>>{};
    } else {
        return std::type_identity<std::remove_cvref_t<Source>>{};
    }
}

template <typename Source>
using sample_type_from_source_t = typename decltype(sample_type_tag<Source>())::type;

template <typename Source> constexpr auto to_sample_source(Source& source) {
    if constexpr (SampleSourceLike<Source>) {
        return source;
    } else {
        using StorageType = std::remove_reference_t<Source>;
        return ReferenceSampleSource<StorageType>{source};
    }
}

template <ProtectionSample T, std::size_t N> struct BakedRules {
    using sample_type = T;
    static constexpr std::size_t rule_count = N;

    std::array<RuleDefinition<T>, N> definitions{};
};

template <typename T, typename Rule>
concept RuleDefinitionLike =
    ProtectionSample<T> &&
    (std::same_as<std::remove_cvref_t<Rule>, RuleDefinition<T>> ||
     std::same_as<std::remove_cvref_t<Rule>, expected<RuleDefinition<T>, RuleConfigError>>);

template <ProtectionSample T> constexpr RuleDefinition<T> unwrap_rule(RuleDefinition<T> rule) {
    return rule;
}

template <ProtectionSample T>
constexpr RuleDefinition<T> unwrap_rule(expected<RuleDefinition<T>, RuleConfigError> rule) {
    return rule.value();
}

template <ProtectionSample T, typename... RuleDefs>
    requires((RuleDefinitionLike<T, RuleDefs> && ...))
constexpr auto bake_rules(RuleDefs... definitions) {
    static_assert(sizeof...(RuleDefs) > 0, "A protection must declare at least one rule");
    return BakedRules<T, sizeof...(RuleDefs)>{
        std::array<RuleDefinition<T>, sizeof...(RuleDefs)>{unwrap_rule<T>(definitions)...}
    };
}

template <typename... Types> struct AreUnique : std::true_type {};

template <typename Type, typename... Rest>
struct AreUnique<Type, Rest...>
    : std::bool_constant<(!std::same_as<Type, Rest> && ...) && AreUnique<Rest...>::value> {};

} // namespace detail

template <std::size_t N> struct FixedString {
    char value[N]{};

    constexpr FixedString(const char (&str)[N]) {
        for (std::size_t index = 0; index < N; ++index) {
            value[index] = str[index];
        }
    }

    constexpr const char* c_str() const { return value; }
    constexpr std::size_t size() const { return N - 1; }
};

template <std::size_t N> FixedString(const char (&)[N]) -> FixedString<N>;

template <FixedString Name, uint16_t Id, auto& Source, std::size_t RuleCount>
struct ProtectionSpec {
    using source_type = std::remove_reference_t<decltype(Source)>;
    using sample_type = detail::sample_type_from_source_t<source_type>;
    using source_model_type = decltype(detail::to_sample_source(Source));

    static constexpr auto name = Name;
    static constexpr auto id = Id;
    static constexpr auto& source = Source;
    static constexpr std::size_t rule_count = RuleCount;

    detail::BakedRules<sample_type, RuleCount> rules{};

    template <class Ctx> consteval void inscribe(Ctx&) const {}
};

template <FixedString Name, uint16_t Id, auto& Source, typename... RuleDefs>
constexpr auto protection(RuleDefs... definitions) {
    using SampleType = detail::sample_type_from_source_t<decltype(Source)>;
    return ProtectionSpec<Name, Id, Source, sizeof...(RuleDefs)>{
        detail::bake_rules<SampleType>(definitions...)
    };
}

template <typename T> struct IsProtectionSpec : std::false_type {};

template <FixedString Name, uint16_t Id, auto& Source, std::size_t RuleCount>
struct IsProtectionSpec<ProtectionSpec<Name, Id, Source, RuleCount>> : std::true_type {};

template <typename T>
concept ProtectionSpecLike = IsProtectionSpec<std::remove_cvref_t<T>>::value;

template <auto&... ProtectionSpecs> class ProtectionEngine {
public:
    static_assert(
        detail::AreUnique<std::remove_cvref_t<decltype(ProtectionSpecs)>...>::value,
        "Duplicate protection declarations must use distinct names or sources"
    );

    static constexpr std::size_t protection_count = sizeof...(ProtectionSpecs);

    template <auto& Spec> struct StorageForSpec {
        using spec_type = std::remove_cvref_t<decltype(Spec)>;
        using type = Protection<
            typename spec_type::sample_type,
            spec_type::rule_count,
            typename spec_type::source_model_type>;
    };

    template <auto& Spec> using storage_for_spec_t = typename StorageForSpec<Spec>::type;

    using Storage = std::tuple<storage_for_spec_t<ProtectionSpecs>...>;

    static void initialize() {
#if defined(HAL_RTC_MODULE_ENABLED) && !defined(SIM_ON)
        Global_RTC::ensure_started();
#endif
        reset();
        initialize_impl(std::make_index_sequence<protection_count>{});
    }

    static void evaluate() { evaluate_impl(std::make_index_sequence<protection_count>{}); }

    template <std::size_t Index> static auto& protection_at() {
        return std::get<Index>(protections);
    }

    template <auto& ProtectionSpec> static auto& protection() {
        return protection_at<spec_index<ProtectionSpec>()>();
    }

    static void reset() { reset_impl(std::make_index_sequence<protection_count>{}); }

private:
    template <auto& ProtectionSpec> static constexpr auto make_protection() {
        using SpecType = std::remove_cvref_t<decltype(ProtectionSpec)>;
        using SampleType = typename SpecType::sample_type;
        using SourceType = typename SpecType::source_model_type;

        return Protection<SampleType, SpecType::rule_count, SourceType>{
            SpecType::name.c_str(),
            SpecType::id,
            detail::to_sample_source(SpecType::source),
            ProtectionSpec.rules.definitions
        };
    }

    template <std::size_t... Indices> static void initialize_impl(std::index_sequence<Indices...>) {
        (std::get<Indices>(protections).initialize(), ...);
    }

    template <std::size_t... Indices> static void evaluate_impl(std::index_sequence<Indices...>) {
        (evaluate_one<Indices>(), ...);
    }

    template <std::size_t... Indices> static void reset_impl(std::index_sequence<Indices...>) {
        (std::get<Indices>(protections).clear_runtime_state(), ...);
    }

    template <std::size_t Index> static void evaluate_one() {
        auto& protection_ref = std::get<Index>(protections);
        const Protections::ProtectionEvaluation evaluation = protection_ref.evaluate();

        publish_edge_events(protection_ref, evaluation);
        if (evaluation.has_active_fault) {
            request_fault_if_due(protection_ref, evaluation);
        }
    }

    template <typename ProtectionType>
    static void publish_edge_events(
        ProtectionType& protection_ref,
        const Protections::ProtectionEvaluation& evaluation
    ) {
        for (std::size_t event_index = 0; event_index < evaluation.event_count; ++event_index) {
            const auto& event = evaluation.events[event_index];
            if (event.state == Protections::RuleState::FAULT) {
                continue;
            }

            Diagnostics::Hub::publish_protection_event(
                protection_ref.get_name(),
                protection_ref.get_measurement_id(),
                event.state,
                event.edge,
                event.snapshot
            );
        }
    }

    template <typename ProtectionType>
    static void request_fault_if_due(
        ProtectionType& protection_ref,
        const Protections::ProtectionEvaluation& evaluation
    ) {
        const uint64_t tick = Scheduler::get_global_tick();
        const uint64_t last_publish_tick = protection_ref.get_last_fault_publish_tick();

        if (last_publish_tick != 0 &&
            tick - last_publish_tick < Protections::Config::notify_delay_in_microseconds) {
            return;
        }

        FaultController::request_fault(FaultCause::protection(
            protection_ref.get_name(),
            protection_ref.get_measurement_id(),
            evaluation.active_fault_edge,
            evaluation.active_fault_snapshot
        ));
        protection_ref.set_last_fault_publish_tick(tick);
    }

    template <auto& ProtectionSpec, std::size_t Index = 0>
    static consteval std::size_t spec_index() {
        if constexpr (Index >= protection_count) {
            static_assert([] { return false; }(), "Protection spec not found");
            return 0;
        } else if constexpr (std::same_as<
                                 std::remove_cvref_t<decltype(ProtectionSpec)>,
                                 std::remove_cvref_t<
                                     decltype(std::get<Index>(std::tie(ProtectionSpecs...)))>>) {
            return Index;
        } else {
            return spec_index<ProtectionSpec, Index + 1>();
        }
    }

    inline static Storage protections{make_protection<ProtectionSpecs>()...};
};

} // namespace Protections
