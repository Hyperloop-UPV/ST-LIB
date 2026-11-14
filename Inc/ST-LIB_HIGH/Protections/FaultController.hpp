#pragma once

#include "C++Utilities/CppUtils.hpp"
#include "ST-LIB_HIGH/Protections/ProtectionTypes.hpp"
#include "StateMachine/StateMachine.hpp"

#ifdef STLIB_ETH
#include "HALAL/Models/Packets/Order.hpp"
#include "HALAL/Models/Packets/OrderProtocol.hpp"
#endif

namespace ST_LIB::TestAccess {
struct FaultController;
}

namespace Protections {
template <auto&...> class ProtectionEngine;
}

class PanicReporter;
class FaultReporter;

namespace FaultConfig {
inline constexpr size_t origin_capacity = Protections::Config::max_name_length;
inline constexpr size_t runtime_message_capacity = 160;
inline constexpr size_t function_capacity = 64;
inline constexpr size_t file_capacity = 96;
} // namespace FaultConfig

enum class FaultCauseKind : uint8_t { PANIC = 0, RUNTIME_FAULT, PROTECTION };

struct FaultRuntimePayload {
    uint32_t line{0};
    bool truncated{false};
    char message[FaultConfig::runtime_message_capacity + 1]{};
    char function_name[FaultConfig::function_capacity + 1]{};
    char file_name[FaultConfig::file_capacity + 1]{};
};

struct FaultProtectionPayload {
    Protections::RuleEdge edge{Protections::RuleEdge::NONE};
    uint16_t measurement_id;
    Protections::RuleSnapshot snapshot{};
};

struct FaultCause {
    FaultCauseKind kind{FaultCauseKind::PANIC};
    char origin[FaultConfig::origin_capacity + 1]{};
    FaultRuntimePayload runtime{};
    FaultProtectionPayload protection_event{};

    static FaultCause
    panic(const char* message, bool truncated, int line, const char* func, const char* file);
    static FaultCause runtime_fault(
        const char* message,
        bool truncated,
        int line,
        const char* func,
        const char* file
    );
    static FaultCause protection(
        const char* protection_name,
        uint16_t measurement_id,
        Protections::RuleEdge edge,
        const Protections::RuleSnapshot& snapshot
    );
};

class FaultController {
public:
    using state_id = uint8_t;
    static constexpr size_t max_runtime_storage = 2048;

    template <typename Policy> static void install_runtime() {
        const bool preserve_preinstalled_fault =
            runtime_storage.machine == nullptr && !runtime_started && faulted && has_latched_cause;
        const FaultCause preserved_cause = latched_cause;

        static_assert(
            requires {
                { Policy::has_operational_machine } -> std::convertible_to<const bool>;
                { Policy::on_fault_enter } -> std::convertible_to<Callback>;
            },
            "Fault policy must expose has_operational_machine and on_fault_enter"
        );

        runtime_started = false;
        faulted = preserve_preinstalled_fault;
        has_latched_cause = preserve_preinstalled_fault;
        latched_cause = preserve_preinstalled_fault ? preserved_cause : FaultCause{};
        reconstruct_runtime_machine<Policy>(
            preserve_preinstalled_fault ? RuntimeState::FAULT : RuntimeState::OPERATIONAL
        );
#ifdef STLIB_ETH
        propagation_targets = {};
        propagation_target_count = 0;
        last_retry_us = 0;
#endif
    }

    static void start();
    static void check_transitions();
    static bool is_faulted();
    static const FaultCause* latched_fault_cause();

#ifdef STLIB_ETH
    static void register_fault_propagation(OrderProtocol* socket, Order* fault_order);
#endif

private:
    friend class PanicReporter;
    friend class FaultReporter;
    template <auto&...> friend class Protections::ProtectionEngine;
    friend struct ST_LIB::TestAccess::FaultController;

    enum class RuntimeState : uint8_t { OPERATIONAL = 0, FAULT = 1 };

    struct RuntimeStorage {
        alignas(std::max_align_t) array<byte, max_runtime_storage> bytes{};
        IStateMachine* machine{nullptr};
        void (*destroy)(IStateMachine*){nullptr};
        void (*start)(IStateMachine*){nullptr};
        void (*rebuild_as_fault)(){nullptr};
    };

    template <typename Policy, RuntimeState InitialState>
    static consteval auto build_runtime_machine() {
        constexpr auto operational_state = make_state(RuntimeState::OPERATIONAL);
        constexpr auto fault_state = make_state(RuntimeState::FAULT);

        if constexpr (Policy::has_operational_machine) {
            auto nested =
                StateMachineHelper::add_nesting(operational_state, Policy::operational_machine);
            auto machine = make_state_machine(
                InitialState,
                StateMachineHelper::add_nested_machines(nested),
                operational_state,
                fault_state
            );
            machine.add_enter_action(&FaultController::on_fault_state_enter, fault_state);
            return machine;
        } else {
            auto machine = make_state_machine(InitialState, operational_state, fault_state);
            machine.add_enter_action(&FaultController::on_fault_state_enter, fault_state);
            return machine;
        }
    }

    template <typename Policy, RuntimeState InitialState> static void emplace_runtime_machine() {
        using RuntimeMachine = decltype(build_runtime_machine<Policy, RuntimeState::OPERATIONAL>());
        static_assert(
            sizeof(RuntimeMachine) <= max_runtime_storage,
            "Fault runtime machine exceeds FaultController storage"
        );
        static_assert(
            alignof(RuntimeMachine) <= alignof(std::max_align_t),
            "Fault runtime machine alignment exceeds FaultController storage alignment"
        );

        constexpr auto runtime_prototype = build_runtime_machine<Policy, InitialState>();
        auto* machine = construct_at(
            reinterpret_cast<RuntimeMachine*>(runtime_storage.bytes.data()),
            runtime_prototype
        );

        runtime_storage.machine = machine;
        runtime_storage.destroy = [](IStateMachine* base) {
            destroy_at(static_cast<RuntimeMachine*>(base));
        };
        runtime_storage.start = [](IStateMachine* base) {
            static_cast<RuntimeMachine*>(base)->start();
        };

        global_machine = machine;
    }

    template <typename Policy> static void reconstruct_runtime_machine(RuntimeState initial_state) {
        reset_runtime_storage();
        if (initial_state == RuntimeState::FAULT) {
            emplace_runtime_machine<Policy, RuntimeState::FAULT>();
        } else {
            emplace_runtime_machine<Policy, RuntimeState::OPERATIONAL>();
        }
        runtime_storage.rebuild_as_fault = []() {
            reconstruct_runtime_machine<Policy>(RuntimeState::FAULT);
        };
        on_fault_enter = Policy::on_fault_enter;
    }

    static void reset_runtime_storage();
    static void publish_fault_diagnostic(const FaultCause& cause);
    static void request_fault(const FaultCause& cause);
    static void on_fault_state_enter();

#ifdef STLIB_ETH
    static void propagate_fault();
    static void retry_pending_fault_propagation();
    static void on_fault_order_received();

    struct FaultPropagationTarget {
        OrderProtocol* socket;
        Order* fault_order;
        bool pending = false;
    };
    static constexpr size_t max_propagation_targets = 8;
    static constexpr uint64_t propagation_retry_period_us = 100'000;
    static array<FaultPropagationTarget, max_propagation_targets> propagation_targets;
    static size_t propagation_target_count;
    static uint64_t last_retry_us;
#endif

    static RuntimeStorage runtime_storage;
    static IStateMachine* global_machine;
    static Callback on_fault_enter;
    static FaultCause latched_cause;
    static bool has_latched_cause;
    static bool faulted;
    static bool runtime_started;
};
