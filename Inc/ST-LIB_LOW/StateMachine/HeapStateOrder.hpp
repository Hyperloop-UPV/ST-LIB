#pragma once
#include "StateMachine/StateOrder.hpp"

template<class StateMachineType>
class HeapStateOrder : public HeapOrder{
public:
	StateMachineType& state_machine;
	typename StateMachineType::state_id state;

    template<class... Types>
	HeapStateOrder(uint16_t id,void(*callback)(void), StateMachineType& state_machine, typename StateMachineType::state_id state,Types*... values) : HeapOrder(id,callback,values...),
    		state_machine(state_machine), state(state) {
    	if(not state_machine.get_states().contains(state)){
    		ErrorHandler("State Machine does not contain state, cannot add StateOrder");
    		return;
    	}
    	else state_machine.get_states()[static_cast<size_t>(state)].add_state_order(id);
    	orders[id] = this;
    }

    void process() override {
    	if (callback != nullptr && state_machine.is_on && state_machine.current_state == state) callback();
    }

    void parse(OrderProtocol* socket, uint8_t* data)override{
    	if(state_machine.is_on && state_machine.current_state == state) HeapOrder::parse(data);
    }
};

template<class StateMachineType, class... Types>
HeapStateOrder(uint16_t, void(*)(void), StateMachineType&, typename StateMachineType::state_id, Types*...) -> HeapStateOrder<StateMachineType>;
