#pragma once

#include "HALAL/Services/Communication/Ethernet/LWIP/TCP/Socket.hpp"

template <size_t BufferLength, class... Types>
    requires NotCallablePack<Types*...>
class ForwardOrder : public StackPacket<BufferLength, Types...>, public Order {
public:
    ForwardOrder(uint16_t id, Socket& forwarder, Types*... values)
        : StackPacket<BufferLength, Types...>(id, values...), forwarding_socket{forwarder} {
        orders[id] = this;
    }

    void process() override { return; }

    void parse(uint8_t* data) override { return; }
    void parse(OrderProtocol* socket, uint8_t* data) override {
        struct pbuf* packet = pbuf_alloc(PBUF_TRANSPORT, get_size(), PBUF_POOL);
        pbuf_take(packet, data, get_size());
        forwarding_socket.tx_packet_buffer.push(packet);
        forwarding_socket.send();
    }
    size_t get_size() override { return StackPacket<BufferLength, Types...>::get_size(); }
    uint16_t get_id() override { return StackPacket<BufferLength, Types...>::get_id(); }

private:
    void set_callback(void (*callback)(void)) override { return; }
    uint8_t* build() override { return StackPacket<BufferLength, Types...>::build(); }
    void set_pointer(size_t index, void* pointer) override {
        StackPacket<BufferLength, Types...>::set_pointer(index, pointer);
    }
    // socket in charge of forwarding the order
    Socket& forwarding_socket;
};

#if __cpp_deduction_guides >= 201606
template <class... Types>
    requires NotCallablePack<Types*...>
ForwardOrder(uint16_t id, Socket& fwd, Types*... values)
    -> ForwardOrder<(!has_container<Types...>::value) * total_sizeof<Types...>::value, Types...>;
#endif
