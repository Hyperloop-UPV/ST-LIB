#pragma once

#include "HALAL/Services/Communication/Ethernet/LWIP/TCP/Socket.hpp"
#include "HALAL/Models/Packets/Order.hpp"

class ForwardOrder : public Order {
public:
    Socket& forwarding_socket;

    ForwardOrder(uint16_t id, Socket& forwarder, void* data = nullptr, size_t data_size = 0)
        : Order(id, nullptr, data, data_size), forwarding_socket(forwarder) {}

    template <typename T>
    ForwardOrder(uint16_t id, Socket& forwarder, T* data_struct)
        : Order(id, nullptr, static_cast<void*>(data_struct), sizeof(T)),
          forwarding_socket(forwarder) {}

    void process() {}

    void parse(OrderProtocol* socket, uint8_t* data) {
        (void)socket;
        struct pbuf* packet = pbuf_alloc(PBUF_TRANSPORT, get_size(), PBUF_POOL);
        if (packet != nullptr) {
            pbuf_take(packet, data, get_size());
            forwarding_socket.tx_packet_buffer.push(packet);
            forwarding_socket.send();
        }
    }
};
