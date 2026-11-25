#pragma once
#include "HALAL/Models/Packets/Packet.hpp"
#include <vector>
class Order;

class OrderProtocol {
public:
    virtual ~OrderProtocol() = default;
    virtual bool send_order(Order& order) = 0;
    static std::vector<OrderProtocol*> sockets;

    friend class ServerSocket;
    friend class Socket;
};
