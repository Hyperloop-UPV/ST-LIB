#pragma once
/*
 * Order.hpp
 *
 *  Recreated on: 24 nov. 2026
 *      Author: aleks
 */

#include <cstdint>
#include <cstddef>
#include <string>
#include "HALAL/Models/Packets/Packet.hpp"

class OrderProtocol; // Forward declaration for network socket interface

constexpr inline uint16_t MaxNumOrders = 50;

class Order : public Packet {
public:
    void (*callback)(void) = nullptr;
    std::string* remote_ip = nullptr;

    inline static Order* OrderPtrWarehouse[MaxNumOrders] = {};
    inline static uint16_t count = 0;

    Order(
        uint16_t id = {},
        void (*callback)(void) = nullptr,
        void* data = nullptr,
        size_t data_size = {}
    )
        : Packet(id, data, data_size), callback(callback) {
        if (count < MaxNumOrders) {
            OrderPtrWarehouse[count++] = this;
        }
    }

    // Auto sizeof(T) deduction
    template <typename T>
    Order(uint16_t id, void (*callback)(void), T* data_struct)
        : Order(id, callback, static_cast<void*>(data_struct), sizeof(T)) {}

    ~Order() = default; // No need for virtual destructor because Order does neither use a Heap or
                        // has dynamic memory structures

    void set_callback(void (*callback)(void)) { this->callback = callback; }

    void process() {
        if (callback != nullptr) {
            callback();
        }
    }

    void parse(OrderProtocol* socket, uint8_t* data) {
        Packet::parse(data); // Delegate payload unpacking to Packet
    }

    void store_ip_order(std::string& ip) { remote_ip = &ip; }

    void reset_for_receive() {}

    // Static methods
    static Order* get_order(uint16_t id) {
        for (uint16_t i = 0; i < count; ++i) {
            if (OrderPtrWarehouse[i]->id == id) {
                return OrderPtrWarehouse[i];
            }
        }
        return nullptr;
    }

    static void process_by_id(uint16_t id) {
        Order* order = get_order(id);
        if (order != nullptr) {
            order->process();
        }
    }

    static void process_data(OrderProtocol* socket, uint8_t* data);
};
