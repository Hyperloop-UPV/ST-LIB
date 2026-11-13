#include <gtest/gtest.h>

#include "ErrorHandler/ErrorHandler.hpp"
#include "HALAL/Services/Diagnostics/Diagnostics.hpp"
#include "HALAL/Services/InfoWarning/InfoWarning.hpp"
#include "HALAL/Services/Time/Scheduler.hpp"
#include "ST-LIB_HIGH/Protections/FaultController.hpp"
#include "ST-LIB_HIGH/Protections/ProtectionEngine.hpp"
#include "ST-LIB_HIGH/Protections/Rules.hpp"
#include "ST-LIB_HIGH/Protections/SampleSource.hpp"
#include "StateMachine/StateMachine.hpp"
#include "TestAccess.hpp"

namespace ST_LIB::TestPanicReporter {
void set_fail_on_error(bool enabled);
void reset();
} // namespace ST_LIB::TestPanicReporter

namespace {

namespace TestAccess = ST_LIB::TestAccess;
namespace TestPanicReporter = ST_LIB::TestPanicReporter;

static_assert(Protections::ReadableSampleSource<SampleSource<float>>);
static_assert(Protections::ReadableSampleSource<SampleSource<volatile float>>);
static_assert(std::same_as<SampleSource<volatile float>::value_type, float>);
static_assert(!Protections::ReadableSampleSource<int>);
static_assert(Protections::FloatingSample<float>);
static_assert(!Protections::FloatingSample<int>);
static_assert(Protections::ComparableSample<float>);
static_assert(!Protections::ComparableSample<const char*>);

class RecordingSink final : public Diagnostics::DiagnosticSink {
public:
    explicit RecordingSink(size_t failures_before_success = 0)
        : failures_before_success(failures_before_success) {}

    bool publish(const Diagnostics::DiagnosticRecord& record) override {
        publish_calls++;
        records.push_back(record);
        if (publish_calls <= failures_before_success) {
            return false;
        }
        return true;
    }

    size_t failures_before_success;
    size_t publish_calls{0};
    vector<Diagnostics::DiagnosticRecord> records{};
};

enum class OperationalState : uint8_t { RUN = 0, HOLD = 1 };

bool transition_to_hold = false;
size_t fault_enter_calls = 0;
size_t operational_hold_enter_count = 0;

static constexpr auto operational_run_state =
    make_state(OperationalState::RUN, Transition<OperationalState>{OperationalState::HOLD, []() {
                                                                       return transition_to_hold;
                                                                   }});
static constexpr auto operational_hold_state = make_state(OperationalState::HOLD);

static inline auto test_operational_machine = []() consteval {
    auto sm =
        make_state_machine(OperationalState::RUN, operational_run_state, operational_hold_state);
    sm.add_enter_action([]() { operational_hold_enter_count++; }, operational_hold_state);
    return sm;
}();

void reset_operational_machine() {
    transition_to_hold = false;
    operational_hold_enter_count = 0;
    test_operational_machine.force_change_state(static_cast<size_t>(OperationalState::RUN));
    test_operational_machine.get_states()[0].unregister_all_timed_actions();
    test_operational_machine.get_states()[1].unregister_all_timed_actions();
}

void on_fault_enter() { fault_enter_calls++; }

inline float monitored_value = 2.0f;
inline constexpr uint16_t measurement_id1 = 6767;
inline constexpr auto monitored_protection =
    Protections::protection<"monitored_value", measurement_id1, monitored_value>(
        Protections::Rules::below(1.0f, 1.5f)
    );
using MonitoredProtectionEngine = Protections::ProtectionEngine<monitored_protection>;

inline volatile float volatile_monitored_value = 2.0f;
inline constexpr uint16_t measurement_id2 = 6967;
inline constexpr auto volatile_monitored_protection =
    Protections::protection<"volatile_monitored_value", measurement_id2, volatile_monitored_value>(
        Protections::Rules::below(1.0f, 1.5f)
    );
using VolatileMonitoredProtectionEngine =
    Protections::ProtectionEngine<volatile_monitored_protection>;

inline float time_value = 0.0f;
inline constexpr uint16_t measurement_id3 = 6769;
inline constexpr auto time_protection =
    Protections::protection<"time_value", measurement_id3, time_value>(
        Protections::Rules::time_accumulation(10.0f, 0.001f)
    );
using TimeProtectionEngine = Protections::ProtectionEngine<time_protection>;

inline float time_reset_value = 0.0f;
inline constexpr uint16_t measurement_id4 = 6969;
inline constexpr auto time_reset_protection =
    Protections::protection<"time_reset_value", measurement_id4, time_reset_value>(
        Protections::Rules::time_accumulation(10.0f, 0.001f)
    );
using TimeResetProtectionEngine = Protections::ProtectionEngine<time_reset_protection>;

static_assert(Protections::ProtectionSpecLike<decltype(monitored_protection)>);

FaultCause make_test_runtime_fault(const char* message) {
    return FaultCause::runtime_fault(message, false, 0, "diagnostics_test", "diagnostics_test.cpp");
}

uint32_t emit_warning_and_return_line() {
    constexpr uint32_t expected_line = __LINE__ + 1;
    WARNING("source location warning");
    return expected_line;
}

struct NoMachinePolicy {
    static constexpr bool has_operational_machine = false;
    static constexpr Callback on_fault_enter = &::on_fault_enter;
};

struct OperationalPolicy {
    static constexpr bool has_operational_machine = true;
    static constexpr auto& operational_machine = test_operational_machine;
    static constexpr Callback on_fault_enter = &::on_fault_enter;
};

class DiagnosticsHubTest : public ::testing::Test {
protected:
    void SetUp() override {
        TestAccess::DiagnosticsHub::clear();
        MonitoredProtectionEngine::reset();
        VolatileMonitoredProtectionEngine::reset();
        TimeProtectionEngine::reset();
        TimeResetProtectionEngine::reset();
        TestAccess::FaultController::clear();
        reset_operational_machine();
        TestPanicReporter::reset();
        fault_enter_calls = 0;
        Scheduler::global_tick_us_ = 0;
        Scheduler_global_timer = TIM2_BASE;

        FaultController::install_runtime<NoMachinePolicy>();
        FaultController::start();
    }
};

TEST_F(DiagnosticsHubTest, KeepsLocalHistoryWhenNoSinksAreRegistered) {
    Diagnostics::DiagnosticRecord record{};
    record.severity = Diagnostics::Severity::WARNING;
    record.category = Diagnostics::Category::RUNTIME_WARNING;
    snprintf(record.origin, sizeof(record.origin), "test");
    snprintf(record.payload.runtime.message, sizeof(record.payload.runtime.message), "local only");
    Diagnostics::Hub::publish(record);

    EXPECT_EQ(TestAccess::DiagnosticsHub::history_size(), 1u);
    EXPECT_EQ(TestAccess::DiagnosticsHub::pending_size(), 0u);
}

TEST_F(DiagnosticsHubTest, HistoryIsReplayedWhenFirstSinkIsInstalled) {
    Diagnostics::DiagnosticRecord record{};
    record.severity = Diagnostics::Severity::WARNING;
    record.category = Diagnostics::Category::RUNTIME_WARNING;
    snprintf(record.origin, sizeof(record.origin), "test");
    snprintf(record.payload.runtime.message, sizeof(record.payload.runtime.message), "replay me");
    Diagnostics::Hub::publish(record);

    auto sink_result = Diagnostics::Hub::emplace_sink<RecordingSink>();
    ASSERT_TRUE(sink_result.has_value());
    auto* sink = *sink_result;

    EXPECT_EQ(TestAccess::DiagnosticsHub::pending_size(), 1u);

    Diagnostics::Hub::flush();
    EXPECT_EQ(sink->publish_calls, 1u);
    EXPECT_EQ(TestAccess::DiagnosticsHub::pending_size(), 0u);
}

TEST_F(DiagnosticsHubTest, RetriesOnlyTheSinkThatFailed) {
    auto stable_sink_result = Diagnostics::Hub::emplace_sink<RecordingSink>();
    auto flaky_sink_result = Diagnostics::Hub::emplace_sink<RecordingSink>(1);
    ASSERT_TRUE(stable_sink_result.has_value());
    ASSERT_TRUE(flaky_sink_result.has_value());
    auto* stable_sink = *stable_sink_result;
    auto* flaky_sink = *flaky_sink_result;

    Diagnostics::DiagnosticRecord record{};
    record.severity = Diagnostics::Severity::WARNING;
    record.category = Diagnostics::Category::RUNTIME_WARNING;
    snprintf(record.origin, sizeof(record.origin), "test");
    snprintf(record.payload.runtime.message, sizeof(record.payload.runtime.message), "retry me");
    Diagnostics::Hub::publish(record);

    Diagnostics::Hub::flush();
    EXPECT_EQ(stable_sink->publish_calls, 1u);
    EXPECT_EQ(flaky_sink->publish_calls, 1u);
    EXPECT_EQ(TestAccess::DiagnosticsHub::pending_size(), 1u);

    Diagnostics::Hub::flush();
    EXPECT_EQ(stable_sink->publish_calls, 1u);
    EXPECT_EQ(flaky_sink->publish_calls, 2u);
    EXPECT_EQ(TestAccess::DiagnosticsHub::pending_size(), 0u);
}

TEST_F(DiagnosticsHubTest, PendingQueueIsBoundedWhenASinkNeverDelivers) {
    auto stuck_sink_result =
        Diagnostics::Hub::emplace_sink<RecordingSink>(std::numeric_limits<size_t>::max());
    ASSERT_TRUE(stuck_sink_result.has_value());

    for (size_t record_index = 0; record_index < Diagnostics::Config::pending_capacity + 5;
         record_index++) {
        Diagnostics::DiagnosticRecord record{};
        record.severity = Diagnostics::Severity::WARNING;
        record.category = Diagnostics::Category::RUNTIME_WARNING;
        snprintf(record.origin, sizeof(record.origin), "test");
        snprintf(
            record.payload.runtime.message,
            sizeof(record.payload.runtime.message),
            "event %zu",
            record_index
        );
        Diagnostics::Hub::publish(record);
    }

    EXPECT_EQ(TestAccess::DiagnosticsHub::pending_size(), Diagnostics::Config::pending_capacity);
}

TEST_F(DiagnosticsHubTest, ReinstallingRuntimeClearsLatchedFaultState) {
    TestAccess::FaultController::request_fault(make_test_runtime_fault("first fault"));
    ASSERT_TRUE(FaultController::is_faulted());
    ASSERT_NE(FaultController::latched_fault_cause(), nullptr);

    FaultController::install_runtime<NoMachinePolicy>();
    FaultController::start();

    EXPECT_FALSE(FaultController::is_faulted());
    EXPECT_EQ(FaultController::latched_fault_cause(), nullptr);
    EXPECT_EQ(fault_enter_calls, 1u);
}

TEST_F(DiagnosticsHubTest, FaultControllerTransitionsOnlyOnce) {
    TestAccess::FaultController::request_fault(make_test_runtime_fault("fault once"));
    TestAccess::FaultController::request_fault(make_test_runtime_fault("fault twice"));

    ASSERT_TRUE(FaultController::is_faulted());
    ASSERT_NE(FaultController::latched_fault_cause(), nullptr);
    EXPECT_EQ(fault_enter_calls, 1u);
    EXPECT_EQ(FaultController::latched_fault_cause()->kind, FaultCauseKind::RUNTIME_FAULT);
    EXPECT_STREQ(FaultController::latched_fault_cause()->runtime.message, "fault once");
}

TEST_F(DiagnosticsHubTest, FaultControllerDelegatesToOperationalMachineWhileOperational) {
    FaultController::install_runtime<OperationalPolicy>();
    reset_operational_machine();
    FaultController::start();

    transition_to_hold = true;
    FaultController::check_transitions();

    EXPECT_EQ(test_operational_machine.get_current_state(), OperationalState::HOLD);
    EXPECT_EQ(operational_hold_enter_count, 1u);
}

TEST_F(DiagnosticsHubTest, FaultBeforeStartStartsRuntimeDirectlyInFault) {
    FaultController::install_runtime<OperationalPolicy>();
    reset_operational_machine();

    TestAccess::FaultController::request_fault(make_test_runtime_fault("fault before start"));

    EXPECT_TRUE(FaultController::is_faulted());
    EXPECT_EQ(fault_enter_calls, 0u);

    FaultController::start();
    transition_to_hold = true;
    FaultController::check_transitions();

    EXPECT_EQ(fault_enter_calls, 1u);
    EXPECT_EQ(test_operational_machine.get_current_state(), OperationalState::RUN);
    EXPECT_EQ(operational_hold_enter_count, 0u);
}

TEST_F(DiagnosticsHubTest, FaultControllerStopsDelegatingAfterFault) {
    FaultController::install_runtime<OperationalPolicy>();
    reset_operational_machine();
    FaultController::start();

    TestAccess::FaultController::request_fault(make_test_runtime_fault("stop delegating"));
    transition_to_hold = true;
    FaultController::check_transitions();

    EXPECT_EQ(test_operational_machine.get_current_state(), OperationalState::RUN);
    EXPECT_EQ(fault_enter_calls, 1u);
}

TEST(DiagnosticsBootstrapTest, PanicBeforeRuntimeInstallationSurvivesBootstrapAndIsDelivered) {
    TestAccess::DiagnosticsHub::clear();
    MonitoredProtectionEngine::reset();
    VolatileMonitoredProtectionEngine::reset();
    TimeProtectionEngine::reset();
    TimeResetProtectionEngine::reset();
    TestAccess::FaultController::clear();
    reset_operational_machine();
    TestPanicReporter::reset();
    TestPanicReporter::set_fail_on_error(false);
    fault_enter_calls = 0;
    Scheduler::global_tick_us_ = 0;
    Scheduler_global_timer = nullptr;

    PANIC("panic before install");

    ASSERT_TRUE(FaultController::is_faulted());
    ASSERT_NE(FaultController::latched_fault_cause(), nullptr);
    EXPECT_EQ(TestAccess::DiagnosticsHub::history_size(), 1u);
    EXPECT_EQ(TestAccess::DiagnosticsHub::pending_size(), 0u);

    auto sink_result = Diagnostics::Hub::emplace_sink<RecordingSink>();
    ASSERT_TRUE(sink_result.has_value());
    auto* sink = *sink_result;

    EXPECT_EQ(TestAccess::DiagnosticsHub::pending_size(), 1u);

    FaultController::install_runtime<NoMachinePolicy>();
    ASSERT_TRUE(FaultController::is_faulted());
    ASSERT_NE(FaultController::latched_fault_cause(), nullptr);

    FaultController::start();
    EXPECT_EQ(fault_enter_calls, 1u);
    EXPECT_EQ(sink->publish_calls, 1u);
    EXPECT_EQ(TestAccess::DiagnosticsHub::pending_size(), 0u);
}

TEST_F(DiagnosticsHubTest, ProtectionEngineEvaluatesRulesAndPublishesSnapshots) {
    auto sink_result = Diagnostics::Hub::emplace_sink<RecordingSink>();
    ASSERT_TRUE(sink_result.has_value());
    auto* sink = *sink_result;

    MonitoredProtectionEngine::initialize();
    monitored_value = 0.5f;
    MonitoredProtectionEngine::evaluate();
    Diagnostics::Hub::flush();

    ASSERT_FALSE(sink->records.empty());
    EXPECT_TRUE(FaultController::is_faulted());
    EXPECT_EQ(sink->records.front().category, Diagnostics::Category::PROTECTION_EVENT);
    EXPECT_EQ(sink->records.front().severity, Diagnostics::Severity::FAULT);
    EXPECT_EQ(sink->records.front().priority, Diagnostics::DiagnosticPriority::URGENT);
    EXPECT_EQ(sink->records.front().payload.protection.state, Protections::RuleState::FAULT);
    EXPECT_EQ(sink->records.front().payload.protection.rule_kind, Protections::RuleKind::BELOW);
}

TEST_F(DiagnosticsHubTest, ProtectionEngineReadsVolatileSource) {
    auto sink_result = Diagnostics::Hub::emplace_sink<RecordingSink>();
    ASSERT_TRUE(sink_result.has_value());
    auto* sink = *sink_result;

    VolatileMonitoredProtectionEngine::initialize();
    volatile_monitored_value = 0.5f;
    VolatileMonitoredProtectionEngine::evaluate();
    Diagnostics::Hub::flush();

    ASSERT_FALSE(sink->records.empty());
    EXPECT_TRUE(FaultController::is_faulted());
    EXPECT_EQ(sink->records.front().category, Diagnostics::Category::PROTECTION_EVENT);
    EXPECT_EQ(sink->records.front().payload.protection.rule_kind, Protections::RuleKind::BELOW);
    EXPECT_FLOAT_EQ(sink->records.front().payload.protection.observed_value.float32_value, 0.5f);
}

TEST_F(DiagnosticsHubTest, TimeAccumulationUsesSchedulerTickForContinuousDuration) {
    auto sink_result = Diagnostics::Hub::emplace_sink<RecordingSink>();
    ASSERT_TRUE(sink_result.has_value());
    auto* sink = *sink_result;

    TimeProtectionEngine::initialize();

    time_value = 12.0f;
    Scheduler::global_tick_us_ = 0;
    TimeProtectionEngine::evaluate();
    EXPECT_FALSE(FaultController::is_faulted());

    Scheduler::global_tick_us_ = 500;
    TimeProtectionEngine::evaluate();
    EXPECT_FALSE(FaultController::is_faulted());

    Scheduler::global_tick_us_ = 1'000;
    TimeProtectionEngine::evaluate();
    Diagnostics::Hub::flush();

    ASSERT_FALSE(sink->records.empty());
    EXPECT_TRUE(FaultController::is_faulted());
    EXPECT_EQ(sink->records.back().category, Diagnostics::Category::PROTECTION_EVENT);
    EXPECT_EQ(
        sink->records.back().payload.protection.rule_kind,
        Protections::RuleKind::TIME_ACCUMULATION
    );
    EXPECT_FLOAT_EQ(sink->records.back().payload.protection.time_window_s, 0.001f);
    EXPECT_FLOAT_EQ(sink->records.back().payload.protection.active_time_s, 0.001f);
}

TEST_F(DiagnosticsHubTest, TimeAccumulationResetsWhenConditionClears) {
    TimeResetProtectionEngine::initialize();

    time_reset_value = 12.0f;
    Scheduler::global_tick_us_ = 0;
    TimeResetProtectionEngine::evaluate();

    Scheduler::global_tick_us_ = 700;
    TimeResetProtectionEngine::evaluate();
    EXPECT_FALSE(FaultController::is_faulted());

    time_reset_value = 0.0f;
    Scheduler::global_tick_us_ = 800;
    TimeResetProtectionEngine::evaluate();
    EXPECT_FALSE(FaultController::is_faulted());

    Scheduler::global_tick_us_ = 1'600;
    TimeResetProtectionEngine::evaluate();
    EXPECT_FALSE(FaultController::is_faulted());

    time_reset_value = 12.0f;
    Scheduler::global_tick_us_ = 1'600;
    TimeResetProtectionEngine::evaluate();

    Scheduler::global_tick_us_ = 2'300;
    TimeResetProtectionEngine::evaluate();
    EXPECT_FALSE(FaultController::is_faulted());
}

TEST_F(DiagnosticsHubTest, PanicPublishesAndEntersFault) {
    auto sink_result = Diagnostics::Hub::emplace_sink<RecordingSink>();
    ASSERT_TRUE(sink_result.has_value());
    auto* sink = *sink_result;
    TestPanicReporter::set_fail_on_error(false);

    PANIC("runtime panic %d", 12);
    Diagnostics::Hub::flush();

    ASSERT_FALSE(sink->records.empty());
    EXPECT_TRUE(FaultController::is_faulted());
    EXPECT_EQ(fault_enter_calls, 1u);
    EXPECT_EQ(sink->records.front().category, Diagnostics::Category::RUNTIME_PANIC);
    EXPECT_EQ(sink->records.front().severity, Diagnostics::Severity::FAULT);
    EXPECT_EQ(sink->records.front().priority, Diagnostics::DiagnosticPriority::URGENT);
}

TEST_F(DiagnosticsHubTest, FaultPublishesAndEntersFault) {
    auto sink_result = Diagnostics::Hub::emplace_sink<RecordingSink>();
    ASSERT_TRUE(sink_result.has_value());
    auto* sink = *sink_result;

    FAULT("runtime fault %d", 12);
    Diagnostics::Hub::flush();

    ASSERT_FALSE(sink->records.empty());
    EXPECT_TRUE(FaultController::is_faulted());
    EXPECT_EQ(fault_enter_calls, 1u);
    EXPECT_EQ(sink->records.front().category, Diagnostics::Category::RUNTIME_FAULT);
    EXPECT_EQ(sink->records.front().severity, Diagnostics::Severity::FAULT);
    EXPECT_EQ(sink->records.front().priority, Diagnostics::DiagnosticPriority::URGENT);
}

TEST_F(DiagnosticsHubTest, WarningDoesNotEnterFault) {
    auto sink_result = Diagnostics::Hub::emplace_sink<RecordingSink>();
    ASSERT_TRUE(sink_result.has_value());
    auto* sink = *sink_result;

    WARNING("runtime warning %d", 3);
    Diagnostics::Hub::flush();

    ASSERT_FALSE(sink->records.empty());
    EXPECT_FALSE(FaultController::is_faulted());
    EXPECT_EQ(sink->records.front().category, Diagnostics::Category::RUNTIME_WARNING);
    EXPECT_EQ(sink->records.front().severity, Diagnostics::Severity::WARNING);
    EXPECT_EQ(sink->records.front().priority, Diagnostics::DiagnosticPriority::NORMAL);
}

TEST_F(DiagnosticsHubTest, InfoPublishesWithoutEnteringFault) {
    auto sink_result = Diagnostics::Hub::emplace_sink<RecordingSink>();
    ASSERT_TRUE(sink_result.has_value());
    auto* sink = *sink_result;

    INFO("runtime info %d", 7);
    Diagnostics::Hub::flush();

    ASSERT_FALSE(sink->records.empty());
    EXPECT_FALSE(FaultController::is_faulted());
    EXPECT_EQ(sink->records.front().category, Diagnostics::Category::RUNTIME_INFO);
    EXPECT_EQ(sink->records.front().severity, Diagnostics::Severity::INFO);
    EXPECT_EQ(sink->records.front().priority, Diagnostics::DiagnosticPriority::NORMAL);
}

TEST_F(DiagnosticsHubTest, RuntimeDiagnosticsCaptureCallerSourceLocation) {
    auto sink_result = Diagnostics::Hub::emplace_sink<RecordingSink>();
    ASSERT_TRUE(sink_result.has_value());
    auto* sink = *sink_result;

    const uint32_t expected_line = emit_warning_and_return_line();
    Diagnostics::Hub::flush();

    ASSERT_FALSE(sink->records.empty());
    EXPECT_EQ(sink->records.front().payload.runtime.line, expected_line);
    EXPECT_NE(
        strstr(sink->records.front().payload.runtime.function_name, "emit_warning_and_return_line"),
        nullptr
    );
}

TEST_F(DiagnosticsHubTest, RejectsInvalidRuleConfigurationsWithoutGlobalSideEffects) {
    auto invalid_rule = Protections::Rules::below(1.0f, 0.5f);
    EXPECT_FALSE(invalid_rule.has_value());
    EXPECT_EQ(invalid_rule.error(), Protections::RuleConfigError::INVALID_WARNING_THRESHOLD);

    auto invalid_time_rule = Protections::Rules::time_accumulation(10.0f, 0.0f);
    EXPECT_FALSE(invalid_time_rule.has_value());
    EXPECT_EQ(invalid_time_rule.error(), Protections::RuleConfigError::INVALID_WINDOW);
}

} // namespace
