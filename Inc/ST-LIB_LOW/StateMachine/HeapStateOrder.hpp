#pragma once
#include "StateMachine/StateOrder.hpp"

class HeapStateOrder : public HeapOrder{
public:
	StateMachine& state_machine;
	StateMachine::state_id state;

    template<class... Types>
	HeapStateOrder(uint16_t id,void(*callback)(void), StateMachine& state_machine, StateMachine::state_id state,Types*... values) : HeapOrder(id,callback,values...),
    		state_machine(state_machine), state(state) {
    	if(not state_machine.get_states().contains(state)){
    		ErrorHandler("State Machine does not contain state, cannot add StateOrder");
    		return;
    	}
    	else state_machine.get_states()[state].add_state_order(id);
    	orders[id] = this;
    }

    void process() override {
    	if (callback != nullptr && state_machine.is_on && state_machine.current_state == state) callback();
    }

    void parse(OrderProtocol* socket, uint8_t* data)override{
    	if(state_machine.is_on && state_machine.current_state == state) HeapOrder::parse(data);
    }
};