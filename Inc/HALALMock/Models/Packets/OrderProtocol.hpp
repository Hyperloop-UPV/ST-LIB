#pragma once
#include "HALALMock/Models/Packets/Packet.hpp"

class Order;

class OrderProtocol{
public:
    virtual bool send_order(Order& order) = 0;
    static vector<OrderProtocol*> sockets;

    friend class ServerSocket;
    friend class Socket;
};
