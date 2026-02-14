#include "StateMachine/StateOrder.hpp"
#include <span>

OrderProtocol* StateOrder::informer_socket = nullptr;
uint16_t StateOrder::state_orders_ids_size = 0;
std::span<uint16_t> StateOrder::state_orders_ids;
StackOrder<0, uint16_t, std::span<uint16_t>> StateOrder::add_state_orders_order(
    StateOrdersID::ADD_STATE_ORDERS,
    &StateOrder::state_orders_ids_size,
    &StateOrder::state_orders_ids
);
StackOrder<0, uint16_t, std::span<uint16_t>> StateOrder::remove_state_orders_order(
    StateOrdersID::REMOVE_STATE_ORDERS,
    &StateOrder::state_orders_ids_size,
    &StateOrder::state_orders_ids
);
