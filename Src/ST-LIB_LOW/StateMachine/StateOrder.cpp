#include "StateMachine/StateOrder.hpp"
#include <span>

OrderProtocol* StateOrder::informer_socket = nullptr;
uint16_t StateOrder::state_orders_ids_size = 0;
vector<uint16_t>* StateOrder::state_orders_ids = nullptr;
StackOrder<0,uint16_t, std::span<uint16_t>> StateOrder::add_state_orders_order(StateOrdersID::ADD_STATE_ORDERS, &StateOrder::state_orders_ids_size, nullptr);
StackOrder<0,uint16_t, std::span<uint16_t>> StateOrder::remove_state_orders_order(StateOrdersID::REMOVE_STATE_ORDERS, &StateOrder::state_orders_ids_size, nullptr);
