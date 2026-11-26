#pragma once

#include "HALAL/Models/IPV4/IPV4.hpp"
#include "HALAL/Models/Packets/Order.hpp"

#ifdef HAL_ETH_MODULE_ENABLED

namespace TcpOrderStreamParser {

inline constexpr size_t MAX_RX_STREAM_BUFFER_BYTES = 8192;

inline void process(OrderProtocol* protocol, IPV4& remote_ip, vector<uint8_t>& stream_buffer) {
    if (stream_buffer.empty()) {
        return;
    }
    size_t parsed_bytes = 0;

    while (stream_buffer.size() - parsed_bytes >= sizeof(uint16_t)) {
        uint8_t* packet_ptr = stream_buffer.data() + parsed_bytes;
        uint16_t order_id = Packet::get_id(packet_ptr);
        Order* order = Order::get_order(order_id);
        if (order == nullptr) {
            parsed_bytes += 1;
            continue;
        }

        const size_t order_size = order->get_size();
        if (order_size > 0) {
            if (order_size < sizeof(uint16_t)) {
                parsed_bytes += 1;
                continue;
            }
            if (stream_buffer.size() - parsed_bytes < order_size) {
                break;
            }
        }

        order->store_ip_order(remote_ip.string_address);
        Order::process_data(protocol, packet_ptr);

        const size_t consumed = order->get_size();
        if (consumed > 0) {
            parsed_bytes += consumed;
        } else {
            parsed_bytes += sizeof(uint16_t);
        }
        order->reset_for_receive();
    }

    if (parsed_bytes > 0) {
        stream_buffer.erase(stream_buffer.begin(), stream_buffer.begin() + parsed_bytes);
    }

    if (stream_buffer.size() > MAX_RX_STREAM_BUFFER_BYTES) {
        const size_t trim_count = stream_buffer.size() - MAX_RX_STREAM_BUFFER_BYTES;
        stream_buffer.erase(stream_buffer.begin(), stream_buffer.begin() + trim_count);
    }
}

} // namespace TcpOrderStreamParser

#endif
