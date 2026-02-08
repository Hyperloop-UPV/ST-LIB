#pragma once
#include <cstdint>
#include "HALAL/Models/Packets/Order.hpp"
#include "HALAL/Models/Packets/OrderProtocol.hpp"
#include "ErrorHandler/ErrorHandler.hpp"
#include <span>

class StateOrder : public Order{
public:
	enum StateOrdersID{
		ADD_STATE_ORDERS = 5,
		REMOVE_STATE_ORDERS = 6
	};
	static OrderProtocol* informer_socket;
	static uint16_t state_orders_ids_size;
	static std::span<uint16_t> state_orders_ids;
	static StackOrder<0,uint16_t, std::span<uint16_t>> add_state_orders_order;
	static StackOrder<0,uint16_t, std::span<uint16_t>> remove_state_orders_order;

	static void set_socket(OrderProtocol& socket){
		informer_socket = &socket;
	}

	static void add_state_orders(std::span<uint16_t> new_ids){
		if(informer_socket == nullptr) {
			ErrorHandler("Informer Socket has not been set");
			return;
		};
		state_orders_ids = new_ids;
		state_orders_ids_size = new_ids.size();
		informer_socket->send_order(add_state_orders_order);
	}

	static void remove_state_orders(std::span<uint16_t> old_ids){
		if(informer_socket == nullptr){
			ErrorHandler("Informer Socket has not been set");
			return;
		}
		state_orders_ids = old_ids;
		state_orders_ids_size = old_ids.size();
		informer_socket->send_order(remove_state_orders_order);
	}
};



