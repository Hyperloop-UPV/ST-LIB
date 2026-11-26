#include "StateMachine/StateOrder.hpp"
#include <span>

OrderProtocol* StateOrder::informer_socket = nullptr;
uint16_t StateOrder::state_orders_ids_size = 0;
std::span<uint16_t> StateOrder::state_orders_ids;
Order StateOrder::add_state_orders_order(
    StateOrdersID::ADD_STATE_ORDERS,
    nullptr,
    &StateOrder::state_orders_ids_size
);
Order StateOrder::remove_state_orders_order(
    StateOrdersID::REMOVE_STATE_ORDERS,
    nullptr,
    &StateOrder::state_orders_ids_size
);
