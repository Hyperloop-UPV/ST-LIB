#pragma once
#include "StateMachine/StateOrder.hpp"

template<class StateMachineType, size_t BufferLength,class... Types> requires NotCallablePack<Types*...>
class StackStateOrder : public StackOrder<BufferLength,Types...>{
public:
	StateMachineType& state_machine;
	typename StateMachineType::state_id state;
    StackStateOrder(uint16_t id,void(*callback)(void), StateMachineType& state_machine, typename StateMachineType::state_id state,Types*... values) : StackOrder<BufferLength,Types...>(id,callback,values...),
    		state_machine(state_machine), state(state) {
    	if(not state_machine.get_states().contains(state)){
    		ErrorHandler("State Machine does not contain state, cannot add StateOrder");
    		return;
    	}
    	else state_machine.get_states()[static_cast<size_t>(state)].add_state_order(id);
    	Order::orders[id] = this;
    }

    void process() override {
    	if (this->callback != nullptr && state_machine.is_on && state_machine.get_current_state_id() == state) this->callback();
    }

    void parse(OrderProtocol* socket, uint8_t* data)override{
    	if(state_machine.is_on && state_machine.get_current_state_id() == state) StackOrder<BufferLength,Types...>::parse(data);
    }
};

#if __cpp_deduction_guides >= 201606
template<class StateMachineType, class... Types> requires NotCallablePack<Types*...>
StackStateOrder(uint16_t id,void(*callback)(void), StateMachineType& state_machine, typename StateMachineType::state_id state,Types*... values)->StackStateOrder<StateMachineType, (!has_container<Types...>::value)*total_sizeof<Types...>::value, Types...>;
#endif
