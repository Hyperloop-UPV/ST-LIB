#pragma once

#include "C++Utilities/CppUtils.hpp"
#include "ST-LIB_HIGH/Protections/ProtectionTypes.hpp"
#include "ST-LIB_HIGH/Protections/ProtectionMessages.hpp"

namespace ST_LIB::TestAccess {
struct DiagnosticsHub;
}

namespace Diagnostics {

namespace Config {
inline constexpr size_t max_sinks = 4;
inline constexpr size_t max_sink_storage = 1024;
inline constexpr size_t history_capacity = 16;
inline constexpr size_t pending_capacity = 16;
inline constexpr size_t origin_capacity = Protections::Config::max_name_length;
inline constexpr size_t runtime_message_capacity = 160;
inline constexpr size_t function_capacity = 64;
inline constexpr size_t file_capacity = 96;
inline constexpr size_t formatted_message_capacity = 320;
} // namespace Config

enum class Severity : uint8_t { INFO = 0, WARNING, FAULT };
enum class DiagnosticPriority : uint8_t { NORMAL = 0, URGENT };
enum class Category : uint8_t {
    RUNTIME_PANIC = 0,
    RUNTIME_FAULT,
    RUNTIME_WARNING,
    RUNTIME_INFO,
    PROTECTION_EVENT
};
enum class RegistrationError : uint8_t { CAPACITY_EXCEEDED = 0, STORAGE_TOO_SMALL };

struct Timestamp {
    bool has_rtc{false};
    uint16_t counter{0};
    uint8_t second{0};
    uint8_t minute{0};
    uint8_t hour{0};
    uint8_t day{0};
    uint8_t month{0};
    uint16_t year{0};
    uint64_t uptime_us{0};
};

struct RuntimeDiagnosticPayload {
    uint32_t line{0};
    bool truncated{false};
    char message[Config::runtime_message_capacity + 1]{};
    char function_name[Config::function_capacity + 1]{};
    char file_name[Config::file_capacity + 1]{};
};

struct ProtectionDiagnosticPayload {
    Protections::RuleKind rule_kind{Protections::RuleKind::BELOW};
    Protections::RuleState state{Protections::RuleState::NORMAL};
    Protections::RuleEdge edge{Protections::RuleEdge::NONE};
    Protections::SampleEncoding sample_encoding{Protections::SampleEncoding::SIGNED};
    Protections::NumericValue observed_value{};
    Protections::NumericValue threshold_a{};
    Protections::NumericValue threshold_b{};
    bool has_threshold_b{false};
    bool uses_warning_threshold{false};
    float time_window_s{0.0f};
    float active_time_s{0.0f};
};

union DiagnosticPayload {
    RuntimeDiagnosticPayload runtime;
    ProtectionDiagnosticPayload protection;

    constexpr DiagnosticPayload() : runtime{} {}
};

struct DiagnosticRecord {
    DiagnosticPriority priority{DiagnosticPriority::NORMAL};
    Severity severity{Severity::INFO};
    Category category{Category::RUNTIME_INFO};
    Timestamp timestamp{};
    char origin[Config::origin_capacity + 1]{};
    DiagnosticPayload payload{};
};

struct RuntimeSourceMetadata {
    int line{0};
    const char* function_name{nullptr};
    const char* file_name{nullptr};
};

class DiagnosticSink {
public:
    virtual ~DiagnosticSink() = default;
    virtual bool publish(const DiagnosticRecord& record) = 0;
};

class DiagnosticFormatter {
public:
    static void describe(const DiagnosticRecord& record, char* buffer, size_t buffer_size);
};

class DiagnosticTimestampProvider {
public:
    static Timestamp capture();
};

namespace RecordFactory {

DiagnosticRecord runtime_panic(
    const char* message,
    bool truncated,
    const RuntimeSourceMetadata& metadata,
    DiagnosticPriority priority = DiagnosticPriority::NORMAL
);

DiagnosticRecord runtime_fault(
    const char* message,
    bool truncated,
    const RuntimeSourceMetadata& metadata,
    DiagnosticPriority priority = DiagnosticPriority::NORMAL
);

DiagnosticRecord runtime_warning(
    const char* message,
    bool truncated,
    const RuntimeSourceMetadata& metadata,
    DiagnosticPriority priority = DiagnosticPriority::NORMAL
);

DiagnosticRecord runtime_info(
    const char* message,
    bool truncated,
    const RuntimeSourceMetadata& metadata,
    DiagnosticPriority priority = DiagnosticPriority::NORMAL
);

DiagnosticRecord protection_event(
    const char* protection_name,
    Protections::RuleState state,
    Protections::RuleEdge edge,
    const Protections::RuleSnapshot& snapshot,
    DiagnosticPriority priority = DiagnosticPriority::NORMAL
);

} // namespace RecordFactory

class Hub {
public:
    template <typename Sink, typename... Args>
    static expected<Sink*, RegistrationError> emplace_sink(Args&&... args) {
        const bool had_no_sinks = sink_count == 0;
        if (sink_count >= Config::max_sinks) {
            return unexpected(RegistrationError::CAPACITY_EXCEEDED);
        }
        if constexpr (sizeof(Sink) > Config::max_sink_storage || alignof(Sink) > alignof(std::max_align_t)) {
            return unexpected(RegistrationError::STORAGE_TOO_SMALL);
        } else {
            SinkStorage& slot = sink_storage[sink_count];
            auto* sink = construct_at(
                reinterpret_cast<Sink*>(slot.bytes.data()),
                std::forward<Args>(args)...
            );
            slot.sink = sink;
            slot.destroy = [](DiagnosticSink* base) { destroy_at(static_cast<Sink*>(base)); };
            sinks[sink_count++] = sink;
            if (had_no_sinks) {
                replay_history_to_pending();
            }
            return sink;
        }
    }

    static void publish(DiagnosticRecord record);
    static void publish_runtime_panic(
        const char* message,
        bool truncated,
        int line,
        const char* func,
        const char* file
    );
    static void publish_runtime_fault(
        const char* message,
        bool truncated,
        int line,
        const char* func,
        const char* file
    );
    static void publish_runtime_warning(
        const char* message,
        bool truncated,
        int line,
        const char* func,
        const char* file
    );
    static void publish_runtime_info(
        const char* message,
        bool truncated,
        int line,
        const char* func,
        const char* file
    );
    static void publish_protection_event(
        const char* protection_name,
        Protections::RuleState state,
        Protections::RuleEdge edge,
        const Protections::RuleSnapshot& snapshot
    );
    static void flush();
    static void flush_urgent();

private:
    friend struct ST_LIB::TestAccess::DiagnosticsHub;

    struct PendingRecord {
        DiagnosticRecord record{};
        uint8_t delivered_mask{0};
    };

    struct SinkStorage {
        alignas(std::max_align_t) array<byte, Config::max_sink_storage> bytes{};
        DiagnosticSink* sink{nullptr};
        void (*destroy)(DiagnosticSink*){nullptr};

        void reset() {
            if (sink != nullptr && destroy != nullptr) {
                destroy(sink);
            }
            sink = nullptr;
            destroy = nullptr;
        }
    };

    static void push_history(const DiagnosticRecord& record);
    static void push_pending(const DiagnosticRecord& record);
    static void replay_history_to_pending();
    static void remove_pending(size_t index);
    static size_t find_oldest_normal_pending();
    static void flush_pending(bool urgent_only);

    static array<SinkStorage, Config::max_sinks> sink_storage;
    static array<DiagnosticSink*, Config::max_sinks> sinks;
    static size_t sink_count;
    static array<DiagnosticRecord, Config::history_capacity> history;
    static size_t history_count;
    static size_t history_next_index;
    static array<PendingRecord, Config::pending_capacity> pending_records;
    static size_t pending_count;
#ifdef STLIB_ETH
    static uint64_t last_urgent_flush_us;
    static uint64_t last_normal_flush_us;
#endif
};

class Runtime {
public:
    static void install_default_sinks();

private:
    friend struct ST_LIB::TestAccess::DiagnosticsHub;

    static bool defaults_installed;
};

} // namespace Diagnostics

#ifdef STLIB_ETH
class OrderProtocol;
namespace Diagnostics {
void install_ethernet_sink(const char* ip, uint16_t port);
}
#endif
