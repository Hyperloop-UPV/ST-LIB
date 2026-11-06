#include "HALAL/Services/Diagnostics/Diagnostics.hpp"
#ifdef STLIB_ETH
#include "HALAL/Services/Time/Scheduler.hpp"
#endif
namespace Diagnostics {
array<Hub::SinkStorage, Config::max_sinks> Hub::sink_storage = {};
array<DiagnosticSink*, Config::max_sinks> Hub::sinks = {};
size_t Hub::sink_count = 0;
array<DiagnosticRecord, Config::history_capacity> Hub::history = {};
size_t Hub::history_count = 0;
size_t Hub::history_next_index = 0;
array<Hub::PendingRecord, Config::pending_capacity> Hub::pending_records = {};
size_t Hub::pending_count = 0;
#ifdef STLIB_ETH
uint64_t Hub::last_urgent_flush_us = 0;
uint64_t Hub::last_normal_flush_us = 0;
#endif
bool Runtime::defaults_installed = false;

namespace {

constexpr const char* runtime_panic_origin = "runtime_panic";
constexpr const char* runtime_fault_origin = "runtime_fault";
constexpr const char* runtime_warning_origin = "runtime_warning";
constexpr const char* runtime_info_origin = "runtime_info";

size_t bounded_strnlen(const char* src, size_t max_length) {
    if (src == nullptr) {
        return 0;
    }

    size_t length = 0;
    while (length < max_length && src[length] != '\0') {
        length++;
    }
    return length;
}

template <size_t Capacity>
void copy_c_string(char (&dst)[Capacity], const char* src, bool* truncated = nullptr) {
    if (Capacity == 0) {
        if (truncated != nullptr) {
            *truncated = true;
        }
        return;
    }

    if (src == nullptr) {
        dst[0] = '\0';
        return;
    }

    const size_t length = bounded_strnlen(src, Capacity - 1);
    memcpy(dst, src, length);
    dst[length] = '\0';
    if (truncated != nullptr) {
        *truncated = *truncated || src[length] != '\0';
    }
}

} // namespace

void Hub::push_history(const DiagnosticRecord& record) {
    history[history_next_index] = record;
    history_next_index = (history_next_index + 1) % Config::history_capacity;
    if (history_count < Config::history_capacity) {
        history_count++;
    }
}

void Hub::push_pending(const DiagnosticRecord& record) {
    if (pending_count == Config::pending_capacity) {
        if (record.priority == DiagnosticPriority::URGENT) {
            const size_t normal_index = find_oldest_normal_pending();
            remove_pending(normal_index == pending_count ? 0 : normal_index);
        } else {
            remove_pending(0);
        }
    }

    pending_records[pending_count].record = record;
    pending_records[pending_count].delivered_mask = 0;
    pending_count++;
}

void Hub::replay_history_to_pending() {
    if (sink_count == 0 || history_count == 0 || pending_count != 0) {
        return;
    }

    const size_t oldest_index = history_count == Config::history_capacity ? history_next_index : 0;

    for (size_t replay_index = 0; replay_index < history_count; ++replay_index) {
        const size_t history_index = (oldest_index + replay_index) % Config::history_capacity;
        push_pending(history[history_index]);
    }
}

void Hub::remove_pending(size_t index) {
    if (index >= pending_count) {
        return;
    }

    for (size_t next = index + 1; next < pending_count; ++next) {
        pending_records[next - 1] = pending_records[next];
    }
    pending_count--;
}

size_t Hub::find_oldest_normal_pending() {
    for (size_t index = 0; index < pending_count; ++index) {
        if (pending_records[index].record.priority == DiagnosticPriority::NORMAL) {
            return index;
        }
    }
    return pending_count;
}

void Hub::publish(DiagnosticRecord record) {
    push_history(record);

    if (sink_count == 0) {
        return;
    }

    push_pending(record);
}

DiagnosticRecord RecordFactory::runtime_fault(
    const char* message,
    bool truncated,
    const RuntimeSourceMetadata& metadata,
    DiagnosticPriority priority
) {
    DiagnosticRecord record{};
    record.priority = priority;
    record.severity = Severity::FAULT;
    record.category = Category::RUNTIME_FAULT;
    record.timestamp = DiagnosticTimestampProvider::capture();
    copy_c_string(record.origin, runtime_fault_origin);
    record.payload.runtime.line = static_cast<uint32_t>(metadata.line < 0 ? 0 : metadata.line);
    record.payload.runtime.truncated = truncated;
    copy_c_string(record.payload.runtime.message, message, &record.payload.runtime.truncated);
    copy_c_string(record.payload.runtime.function_name, metadata.function_name);
    copy_c_string(record.payload.runtime.file_name, metadata.file_name);
    return record;
}

DiagnosticRecord RecordFactory::runtime_panic(
    const char* message,
    bool truncated,
    const RuntimeSourceMetadata& metadata,
    DiagnosticPriority priority
) {
    DiagnosticRecord record{};
    record.priority = priority;
    record.severity = Severity::FAULT;
    record.category = Category::RUNTIME_PANIC;
    record.timestamp = DiagnosticTimestampProvider::capture();
    copy_c_string(record.origin, runtime_panic_origin);
    record.payload.runtime.line = static_cast<uint32_t>(metadata.line < 0 ? 0 : metadata.line);
    record.payload.runtime.truncated = truncated;
    copy_c_string(record.payload.runtime.message, message, &record.payload.runtime.truncated);
    copy_c_string(record.payload.runtime.function_name, metadata.function_name);
    copy_c_string(record.payload.runtime.file_name, metadata.file_name);
    return record;
}

DiagnosticRecord RecordFactory::runtime_warning(
    const char* message,
    bool truncated,
    const RuntimeSourceMetadata& metadata,
    DiagnosticPriority priority
) {
    DiagnosticRecord record{};
    record.priority = priority;
    record.severity = Severity::WARNING;
    record.category = Category::RUNTIME_WARNING;
    record.timestamp = DiagnosticTimestampProvider::capture();
    copy_c_string(record.origin, runtime_warning_origin);
    record.payload.runtime.line = static_cast<uint32_t>(metadata.line < 0 ? 0 : metadata.line);
    record.payload.runtime.truncated = truncated;
    copy_c_string(record.payload.runtime.message, message, &record.payload.runtime.truncated);
    copy_c_string(record.payload.runtime.function_name, metadata.function_name);
    copy_c_string(record.payload.runtime.file_name, metadata.file_name);
    return record;
}

DiagnosticRecord RecordFactory::runtime_info(
    const char* message,
    bool truncated,
    const RuntimeSourceMetadata& metadata,
    DiagnosticPriority priority
) {
    DiagnosticRecord record{};
    record.priority = priority;
    record.severity = Severity::INFO;
    record.category = Category::RUNTIME_INFO;
    record.timestamp = DiagnosticTimestampProvider::capture();
    copy_c_string(record.origin, runtime_info_origin);
    record.payload.runtime.line = static_cast<uint32_t>(metadata.line < 0 ? 0 : metadata.line);
    record.payload.runtime.truncated = truncated;
    copy_c_string(record.payload.runtime.message, message, &record.payload.runtime.truncated);
    copy_c_string(record.payload.runtime.function_name, metadata.function_name);
    copy_c_string(record.payload.runtime.file_name, metadata.file_name);
    return record;
}

DiagnosticRecord RecordFactory::protection_event(
    const char* protection_name,
    Protections::RuleState state,
    Protections::RuleEdge edge,
    const Protections::RuleSnapshot& snapshot,
    DiagnosticPriority priority
) {
    DiagnosticRecord record{};
    record.priority = priority;
    switch (state) {
    case Protections::RuleState::FAULT:
        record.severity = Severity::FAULT;
        break;
    case Protections::RuleState::WARNING:
        record.severity = Severity::WARNING;
        break;
    case Protections::RuleState::NORMAL:
        record.severity = Severity::INFO;
        break;
    }

    record.category = Category::PROTECTION_EVENT;
    record.timestamp = DiagnosticTimestampProvider::capture();
    copy_c_string(record.origin, protection_name);
    record.payload.protection.rule_kind = snapshot.kind;
    record.payload.protection.state = state;
    record.payload.protection.edge = edge;
    record.payload.protection.sample_encoding = snapshot.sample_encoding;
    record.payload.protection.observed_value = snapshot.observed_value;
    record.payload.protection.threshold_a = snapshot.threshold_a;
    record.payload.protection.threshold_b = snapshot.threshold_b;
    record.payload.protection.has_threshold_b = snapshot.has_threshold_b;
    record.payload.protection.uses_warning_threshold = snapshot.uses_warning_threshold;
    record.payload.protection.time_window_s = snapshot.time_window_s;
    record.payload.protection.active_time_s = snapshot.active_time_s;
    return record;
}

void Hub::publish_runtime_fault(
    const char* message,
    bool truncated,
    int line,
    const char* func,
    const char* file
) {
    publish(
        RecordFactory::runtime_fault(message, truncated, RuntimeSourceMetadata{line, func, file})
    );
}

void Hub::publish_runtime_panic(
    const char* message,
    bool truncated,
    int line,
    const char* func,
    const char* file
) {
    publish(
        RecordFactory::runtime_panic(message, truncated, RuntimeSourceMetadata{line, func, file})
    );
}

void Hub::publish_runtime_warning(
    const char* message,
    bool truncated,
    int line,
    const char* func,
    const char* file
) {
    publish(
        RecordFactory::runtime_warning(message, truncated, RuntimeSourceMetadata{line, func, file})
    );
}

void Hub::publish_runtime_info(
    const char* message,
    bool truncated,
    int line,
    const char* func,
    const char* file
) {
    publish(RecordFactory::runtime_info(message, truncated, RuntimeSourceMetadata{line, func, file})
    );
}

void Hub::publish_protection_event(
    const char* protection_name,
    Protections::RuleState state,
    Protections::RuleEdge edge,
    const Protections::RuleSnapshot& snapshot
) {
    publish(RecordFactory::protection_event(protection_name, state, edge, snapshot));
}

void Hub::flush_pending(bool urgent_only) {
    const uint8_t target_mask = sink_count == 0 ? 0 : static_cast<uint8_t>((1u << sink_count) - 1u);
#ifdef STLIB_ETH
    const uint64_t now = Scheduler::get_global_tick();
    uint64_t& last_flush = urgent_only ? last_urgent_flush_us : last_normal_flush_us;
#endif

    for (size_t record_index = 0; record_index < pending_count;) {
        PendingRecord& pending_record = pending_records[record_index];
        if (urgent_only && pending_record.record.priority != DiagnosticPriority::URGENT) {
            record_index++;
            continue;
        }

#ifdef STLIB_ETH
        if (last_flush != 0 && now - last_flush < 100'000) {
            record_index++;
            continue;
        }
#endif

        for (size_t sink_index = 0; sink_index < sink_count; ++sink_index) {
            const uint8_t sink_mask = static_cast<uint8_t>(1u << sink_index);
            if ((pending_record.delivered_mask & sink_mask) != 0u) {
                continue;
            }
            if (sinks[sink_index] != nullptr && sinks[sink_index]->publish(pending_record.record)) {
                pending_record.delivered_mask |= sink_mask;
#ifdef STLIB_ETH
                last_flush = now;
#endif
            }
        }

        if (pending_record.delivered_mask == target_mask) {
            remove_pending(record_index);
        } else {
            record_index++;
        }
    }
}

void Hub::flush_urgent() { flush_pending(true); }

void Hub::flush() {
    flush_urgent();
    flush_pending(false);
}

} // namespace Diagnostics
