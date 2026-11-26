#include "HALAL/Models/Packets/Order.hpp"

void Order::process_data(OrderProtocol* socket, uint8_t* data) {
    uint16_t id = Packet::get_id(data);
    Order* order = Order::get_order(id);
    if (order != nullptr) {
        order->parse(socket, data);
        order->process();
    }
}
