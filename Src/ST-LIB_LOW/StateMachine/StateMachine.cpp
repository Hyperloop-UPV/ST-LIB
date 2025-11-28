/*
 * Created by Alejandro
 */

#include "StateMachine/StateMachine.hpp"
#include "ErrorHandler/ErrorHandler.hpp"

#ifdef SIM_ON
#include "HALALMock/Services/SharedMemory/SharedMemory.hpp"
#endif

void State::enter() {
	for (function<void()>& action : on_enter_actions) {
		action();
	}
	// if(state_orders_ids.size() != 0) StateOrder::add_state_orders(state_orders_ids);
}

void State::exit() {
	for (function<void()>& action : on_exit_actions) {
		action();
	}
	// if(state_orders_ids.size() != 0) StateOrder::remove_state_orders(state_orders_ids);
}

void State::unregister_timed_action(TimedAction* timed_action){
	switch (timed_action->alarm_precision) {
		case LOW_PRECISION:
			Time::unregister_low_precision_alarm(timed_action->id);
			break;
		case MID_PRECISION:
			Time::unregister_mid_precision_alarm(timed_action->id);
			break;
		case HIGH_PRECISION:
			Time::unregister_high_precision_alarm(timed_action->id);
			break;
		default:
			ErrorHandler("Alarm Precision Type does not exist, AlarmType: %d", timed_action->alarm_precision);
			return;
			break;
	}
	timed_action->is_on = false;
}

void State::erase_timed_action(TimedAction* timed_action){
	if(timed_action->is_on) unregister_timed_action(timed_action);
	cyclic_actions.erase(find_if(cyclic_actions.begin(), cyclic_actions.end(), [&](TimedAction& other){
		if(&other == timed_action) return true;
		return false;
	}));
}

void State::unregister_all_timed_actions(){
	for(TimedAction& timed_action : cyclic_actions){
		if(!timed_action.is_on) continue;
		switch(timed_action.alarm_precision){
		case LOW_PRECISION:
			Time::unregister_low_precision_alarm(timed_action.id);
			break;
		case MID_PRECISION:
			Time::unregister_mid_precision_alarm(timed_action.id);
			break;
		case HIGH_PRECISION:
			Time::unregister_high_precision_alarm(timed_action.id);
			break;
		default:
			ErrorHandler("Cannot unregister timed action with erroneus alarm precision, Alarm Precision Type: %d", timed_action.alarm_precision);
			return;
			break;
		}
		timed_action.is_on = false;
	}
}

void State::register_all_timed_actions(){
	for(TimedAction& timed_action : cyclic_actions){
		switch(timed_action.alarm_precision){
		case LOW_PRECISION:
			timed_action.id = Time::register_low_precision_alarm(timed_action.period, timed_action.action);
			break;
		case MID_PRECISION:
			timed_action.id = Time::register_mid_precision_alarm(timed_action.period, timed_action.action);
			break;
		case HIGH_PRECISION:
			timed_action.id = Time::register_high_precision_alarm(timed_action.period, timed_action.action);
			break;
		default:
			ErrorHandler("Cannot register timed action with erroneus alarm precision, Alarm Precision Type: %d", timed_action.alarm_precision);
			return;
			break;
		}
		timed_action.is_on = true;
	}
}

void State::add_state_order(uint16_t id){
	state_orders_ids.push_back(id);
}

#ifdef SIM_ON
StateMachine::StateMachine(){
	state_machine_id_in_shm = ++(*SharedMemory::state_machine_count);
	SharedMemory::update_current_state(state_machine_id_in_shm,initial_state);
}
#else
StateMachine::StateMachine(){}
#endif

/**
 * This is a constructor for a StateMachine object that initializes the initial state and creates a
 * State object for it.
 * 
 * @param initial_state The initial state parameter is an unsigned 8-bit integer that represents the
 * starting state of the state machine.
 */
StateMachine::StateMachine(uint8_t initial_state) :
	initial_state(initial_state), current_state(initial_state) {
	add_state(initial_state);
	enter_state(initial_state);

	#ifdef SIM_ON
		state_machine_id_in_shm = ++(*SharedMemory::state_machine_count);
		SharedMemory::update_current_state(state_machine_id_in_shm,initial_state);
	#endif
}

/**
 * The function adds a state to a state machine and sets it as the initial and current state if it is
 * the first state added.
 * 
 * @param state The state to be added to the state machine.
 * 
 * @return The function does not return anything. It has a void return type.
 */
void StateMachine::add_state(uint8_t state) {
	if (states.contains(state)) {
		ErrorHandler("The state %d is already added", state);
		return;
	}

	if (states.empty()) {
		initial_state = state;
		current_state = state;
	}
	states[state];
}

/**
 * This function adds an action to be executed when entering the current state of a state machine.
 * 
 * @param action The parameter "action" is a function pointer to a function that takes no arguments and
 * returns nothing (void). It represents an action that should be executed when the current state of
 * the state machine is entered.
 */
void StateMachine::add_enter_action(function<void()> action) {
	if (not current_state) {
		ErrorHandler("The state machine does not have a current state");
		return;
	}
	states[current_state].on_enter_actions.push_back(action);
}

/**
 * This function adds an action to be executed when entering a specific state in a state machine.
 * 
 * @param action The action parameter is a function pointer to a function that takes no arguments and
 * returns nothing. This function will be executed when the state machine transitions to the state
 * specified by the state parameter.
 * @param state The state parameter is an unsigned 8-bit integer that represents the state to which the
 * enter action is being added.
 */
void StateMachine::add_enter_action(function<void()> action, uint8_t state) {
	if (not states.contains(state)) {
		ErrorHandler("The state %d is not added to the state machine", state);
		return;
	}
	states[state].on_enter_actions.push_back(action);
}

/**
 * This function adds an exit action to the current state of a state machine.
 * 
 * @param action `action` is a function pointer to a function that takes no arguments and returns
 * nothing (`void`). It represents an action that should be executed when the state machine exits the
 * current state and transitions to a new state. The `add_exit_action` function adds this action to the
 * list of exit actions for
 */
void StateMachine::add_exit_action(function<void()> action) {
	if (not current_state) {
		ErrorHandler("The state machine does not have a current state");
		return;
	}
	states[current_state].on_exit_actions.push_back(action);
}

/**
 * This function adds an exit action to a specific state in a state machine.
 * 
 * @param action The action parameter is a function pointer to a function that takes no arguments and
 * returns nothing. This function represents the action that will be executed when the state machine
 * exits the specified state.
 * @param state The state parameter is an unsigned 8-bit integer that represents the state to which the
 * exit action is being added.
 */
void StateMachine::add_exit_action(function<void()> action, uint8_t state) {
	if (not states.contains(state)) {
		ErrorHandler("The state %d is not added to the state machine");
		return;
	}
	states[state].on_exit_actions.push_back(action);
}

/**
 * This function adds a transition between two states in a state machine.
 * 
 * @param old_state The current state of the state machine before the transition occurs. It is
 * represented as an unsigned 8-bit integer (uint8_t).
 * @param new_state The new state to transition to. It is of type uint8_t.
 * @param transition The parameter "transition" is a function pointer to a boolean function that takes
 * no arguments. This function represents the condition that must be met in order for the state machine
 * to transition from the old_state to the new_state. If the condition is true, the transition will
 * occur. If the condition is false
 */
void StateMachine::add_transition(uint8_t old_state, uint8_t new_state,
		function<bool()> transition) {
	if (not states.contains(old_state)) {
		ErrorHandler("The state %d is not added to the state machine", old_state);
	}

	if(not states.contains(new_state)) {
		ErrorHandler("The state %d is not added to the state machine", new_state);
		return;
	}

	transitions[old_state][new_state] = transition;
}

/**
 * The function adds a nested state machine to the current state machine and clears any timed actions
 * associated with the nested state machine's current state if it is not the current state.
 * 
 * @param state_machine A reference to a StateMachine object that is being added as a nested state
 * machine to the current StateMachine object.
 * @param state The state to which the nested state machine is being added.
 */
void StateMachine::add_state_machine(StateMachine& state_machine, uint8_t state) {
	if(nested_state_machine.contains(state)){
		ErrorHandler("Only one Nested State Machine can be added per state, tried to add to state: %d", state);
		return;
	}

	if(not state_machine.states[state_machine.current_state].cyclic_actions.empty()){
		ErrorHandler("Nested State Machine current state has actions registered, must be empty until nesting");
	}

	nested_state_machine[state] = &state_machine;

	if (current_state != state) {
		state_machine.is_on = false;
	}
}

/**
 * The function checks for state transitions and changes the current state if necessary, and also
 * checks for transitions in a nested state machine if applicable.
 */
void StateMachine::check_transitions() {
	for (auto const& state_transition : transitions[current_state]) {
		if (state_transition.second()) {
			force_change_state(state_transition.first);
		}
	}

	if (nested_state_machine.contains(current_state)) {
		nested_state_machine[current_state]->check_transitions();
	}
}

void StateMachine::force_change_state(uint8_t new_state) {
	if (not states.contains(new_state)) {
		ErrorHandler("The state %d is not added to the state machine", new_state);
		return;
	}

    if(current_state == new_state) return;

	unregister_all_timed_actions(current_state);
	exit_state(current_state);
	
	current_state = new_state;

	enter_state(current_state);
	register_all_timed_actions(current_state);

	#ifdef SIM_ON
		SharedMemory::update_current_state(state_machine_id_in_shm,current_state);
	#endif
}

void StateMachine::remove_cyclic_action(TimedAction* action) {
	if (not states.contains(current_state)) {
		ErrorHandler("The state %d is not added to the state machine", current_state);
		return;
	}

	states[current_state].erase_timed_action(action);
}

void StateMachine::remove_cyclic_action(TimedAction* action, uint8_t state) {
	if (not states.contains(state)) {
		ErrorHandler("The state %d is not added to the state machine", state);
		return;
	}

	states[state].erase_timed_action(action);
}



void StateMachine::enter_state(state_id state) {
	states[state].enter();

	if (nested_state_machine.contains(state)) {
		StateMachine* nested_sm = nested_state_machine[state];
		nested_sm->is_on = true;
		nested_sm->enter_state(nested_sm->current_state);
	}
}

void StateMachine::exit_state(state_id state) {
	states[state].exit();

	if (nested_state_machine.contains(state)) {
		StateMachine* nested_sm = nested_state_machine[state];
		nested_sm->is_on = false;
		nested_sm->exit_state(nested_sm->current_state);
		nested_sm->current_state = nested_sm->initial_state;
	}
}

void StateMachine::register_all_timed_actions(state_id state) {
	states[state].register_all_timed_actions();

	if (nested_state_machine.contains(current_state)) {
		StateMachine* nested_sm = nested_state_machine[current_state];
		nested_sm->states[nested_sm->current_state].register_all_timed_actions();
	}
}

void StateMachine::unregister_all_timed_actions(state_id state) {
	states[state].unregister_all_timed_actions();

	if (nested_state_machine.contains(current_state)) {
		StateMachine* nested_sm = nested_state_machine[current_state];
		nested_sm->states[nested_sm->current_state].unregister_all_timed_actions();
	}
}

unordered_map<StateMachine::state_id, State>& StateMachine::get_states(){
	return states;
}

void StateMachine::refresh_state_orders(){
	// if(states[current_state].state_orders_ids.size() != 0) StateOrder::add_state_orders(states[current_state].state_orders_ids);
}

#ifdef SIM_ON
uint8_t StateMachine::get_id_in_shm(){
	return state_machine_id_in_shm;
}
#endif