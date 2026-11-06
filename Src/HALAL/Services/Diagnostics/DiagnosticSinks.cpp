#include "HALAL/Services/Diagnostics/Diagnostics.hpp"
#include "ST-LIB_HIGH/Protections/ProtectionMessages.hpp"
#include "ST-LIB_HIGH/Protections/ProtectionTypes.hpp"

#if defined(HAL_UART_MODULE_ENABLED) && !defined(SIM_ON)
#include "HALAL/Services/Communication/UART/UART.hpp"
#endif

#ifdef STLIB_ETH
#include "HALAL/Models/Packets/Order.hpp"
#endif

namespace Diagnostics {

namespace {

constexpr uint16_t DIAGNOSTIC_FAULT_ORDER_ID = 1555;
constexpr uint16_t DIAGNOSTIC_WARNING_ORDER_ID = 2555;
constexpr uint16_t DIAGNOSTIC_OK_ORDER_ID = 3000;
constexpr uint8_t DIAGNOSTIC_CHAR_TYPE = 3;
constexpr uint8_t DIAGNOSTIC_ERROR_HANDLER_KIND = 5;
constexpr uint8_t DIAGNOSTIC_WARNING_KIND = 7;
constexpr size_t transport_buffer_capacity = 2 + 1 + 1 + 2 + 1 + 1 + 1 + 1 + 1 + 2 +
                                             Config::origin_capacity + 1 +
                                             Config::formatted_message_capacity + 1;

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

template <size_t Capacity> void copy_c_string(char (&dst)[Capacity], const char* src) {
    if (Capacity == 0) {
        return;
    }

    if (src == nullptr) {
        dst[0] = '\0';
        return;
    }

    const size_t length = bounded_strnlen(src, Capacity - 1);
    memcpy(dst, src, length);
    dst[length] = '\0';
}

#if defined(HAL_UART_MODULE_ENABLED) && !defined(SIM_ON)
class UartDiagnosticSink final : public DiagnosticSink {
public:
    bool publish(const DiagnosticRecord& record) override {
        if (!UART::printf_ready) {
            return false;
        }

        char description[Config::formatted_message_capacity + 1]{};
        DiagnosticFormatter::describe(record, description, sizeof(description));
        printf("%u: %s%s", std::to_underlying(record.severity), description, endl);
        return true;
    }
};
#endif

#ifdef STLIB_ETH
class DiagnosticTransportOrder final : public Order {
public:
    [[maybe_unused]] static inline OrderProtocol* forward_target = nullptr;
    DiagnosticTransportOrder() {
        Packet::packets[DIAGNOSTIC_FAULT_ORDER_ID] = this;
        Packet::packets[DIAGNOSTIC_WARNING_ORDER_ID] = this;
        Packet::packets[DIAGNOSTIC_OK_ORDER_ID] = this;
        orders[DIAGNOSTIC_FAULT_ORDER_ID] = this;
        orders[DIAGNOSTIC_WARNING_ORDER_ID] = this;
        orders[DIAGNOSTIC_OK_ORDER_ID] = this;
    }

    void set_record(const DiagnosticRecord& record, const char* description) {
        id = id_for(record.severity);
        kind = kind_for(record.severity);
        copy_c_string(origin, record.origin);
        copy_c_string(message, description);
        counter = record.timestamp.counter;
        second = record.timestamp.second;
        minute = record.timestamp.minute;
        hour = record.timestamp.hour;
        day = record.timestamp.day;
        month = record.timestamp.month;
        year = record.timestamp.year;
        size = encoded_size();
    }

    void set_callback(void (*callback)(void)) override { this->callback = callback; }

    void parse(OrderProtocol* socket, uint8_t* data) override {
        (void)socket;
        if (data == nullptr)
            return;

        memcpy(&id, data, sizeof(id));

        size_t off = sizeof(id) + sizeof(uint8_t);
        kind = data[off++];

        size_t len = bounded_strnlen(reinterpret_cast<char*>(data + off), sizeof(origin) - 1);
        memcpy(origin, data + off, len);
        origin[len] = '\0';
        off += len + 1;

        len = bounded_strnlen(reinterpret_cast<char*>(data + off), sizeof(message) - 1);
        memcpy(message, data + off, len);
        message[len] = '\0';
        off += len + 1;

        memcpy(
            &counter,
            data + off,
            sizeof(counter) + sizeof(second) + sizeof(minute) + sizeof(hour) + sizeof(day) +
                sizeof(month) + sizeof(year)
        );
        size = off + sizeof(counter) + sizeof(second) + sizeof(minute) + sizeof(hour) +
               sizeof(day) + sizeof(month) + sizeof(year);
    }

    void process() override {
        switch (id) {
        case DIAGNOSTIC_FAULT_ORDER_ID:
            Hub::publish_runtime_fault(message, false, 0, origin, "");
            break;
        case DIAGNOSTIC_WARNING_ORDER_ID:
            Hub::publish_runtime_warning(message, false, 0, origin, "");
            break;
        case DIAGNOSTIC_OK_ORDER_ID:
            Hub::publish_runtime_info(message, false, 0, origin, "");
            break;
        }
    }

    uint8_t* build() override {
        auto bytes = span<byte>(buffer.data(), buffer.size());
        size_t offset = 0;

        write(bytes, offset, id);
        write(bytes, offset, DIAGNOSTIC_CHAR_TYPE);
        write(bytes, offset, kind);
        write_string(bytes, offset, origin);
        write_string(bytes, offset, message);
        write(bytes, offset, counter);
        write(bytes, offset, second);
        write(bytes, offset, minute);
        write(bytes, offset, hour);
        write(bytes, offset, day);
        write(bytes, offset, month);
        write(bytes, offset, year);

        size = offset;
        return reinterpret_cast<uint8_t*>(buffer.data());
    }

    size_t get_size() override { return size; }

    void reset_for_receive() { size = 0; }

    uint16_t get_id() override { return id; }

    void set_pointer(size_t index, void* pointer) override {
        (void)index;
        (void)pointer;
    }

private:
    size_t encoded_size() const {
        return sizeof(id) + sizeof(DIAGNOSTIC_CHAR_TYPE) + sizeof(kind) +
               bounded_strnlen(origin, sizeof(origin)) + 1 +
               bounded_strnlen(message, sizeof(message)) + 1 + sizeof(counter) + sizeof(second) +
               sizeof(minute) + sizeof(hour) + sizeof(day) + sizeof(month) + sizeof(year);
    }

    static uint16_t id_for(Severity severity) {
        switch (severity) {
        case Severity::FAULT:
            return DIAGNOSTIC_FAULT_ORDER_ID;
        case Severity::WARNING:
            return DIAGNOSTIC_WARNING_ORDER_ID;
        case Severity::INFO:
            return DIAGNOSTIC_OK_ORDER_ID;
        }

        return DIAGNOSTIC_OK_ORDER_ID;
    }

    static uint8_t kind_for(Severity severity) {
        return severity == Severity::FAULT ? DIAGNOSTIC_ERROR_HANDLER_KIND
                                           : DIAGNOSTIC_WARNING_KIND;
    }

    template <typename Value> static void write(span<byte> bytes, size_t& offset, Value value) {
        memcpy(bytes.data() + offset, &value, sizeof(Value));
        offset += sizeof(Value);
    }

    static void write_string(span<byte> bytes, size_t& offset, const char* text) {
        const size_t length = bounded_strnlen(text, bytes.size() - offset - 1);
        memcpy(bytes.data() + offset, text, length);
        offset += length;
        bytes[offset++] = byte{0};
    }

    uint16_t id;
    void (*callback)(void) = nullptr;
    uint8_t kind{DIAGNOSTIC_WARNING_KIND};
    char origin[Config::origin_capacity + 1]{};
    char message[Config::formatted_message_capacity + 1]{};
    uint16_t counter{0};
    uint8_t second{0};
    uint8_t minute{0};
    uint8_t hour{0};
    uint8_t day{0};
    uint8_t month{0};
    uint16_t year{0};
    size_t size{0};
    array<byte, transport_buffer_capacity> buffer{};
};

class OrderProtocolDiagnosticSink final : public DiagnosticSink {
public:
    explicit OrderProtocolDiagnosticSink(const char* ip = "192.168.0.9", uint16_t port = 31) {}
    bool publish(const DiagnosticRecord& record) override {
        auto package = ST_LIB::ProtectionADJv3Packet::make(
            Order::last_order_id,
            record.payload.protection.rule_kind,
            record.payload.protection.sample_encoding
        );
        bool delivered = ST_LIB::ProtectionADJv3Packet::tcp_client_send(
            package,
            &record.payload.protection.observed_value
        );
        return delivered;
    }
};
#endif

} // namespace

void Runtime::install_default_sinks() {
    if (defaults_installed) {
        return;
    }

#if defined(HAL_UART_MODULE_ENABLED) && !defined(SIM_ON)
    (void)Hub::emplace_sink<UartDiagnosticSink>();
#endif

    defaults_installed = true;
}

#ifdef STLIB_ETH
void install_ethernet_sink(const char* ip = "192.168.0.9", uint16_t port = 31) {
    (void)Hub::emplace_sink<OrderProtocolDiagnosticSink>(ip, port);
}
#endif

} // namespace Diagnostics
