/*
 * Created by Alejandro
 */

#pragma once
#include "C++Utilities/CppUtils.hpp"
#include "HALAL/HALAL.hpp"
#include "ErrorHandler/ErrorHandler.hpp"
#include "StateMachine/StateOrder.hpp"

#ifdef SIM_ON
#include "HALALMock/Services/SharedMemory/SharedMemory.hpp"
#endif


using ms = std::chrono::milliseconds;
using us = std::chrono::microseconds;
using s  = std::chrono::seconds;

enum AlarmType{
	LOW_PRECISION,
	MID_PRECISION,
	HIGH_PRECISION
};

class TimedAction {
public:
	function<void()> action;
	uint32_t period;
	AlarmType alarm_precision;
	uint8_t id = -1;
	bool is_on = false;

	TimedAction() = default;
};

class State {
public:
	vector<TimedAction> cyclic_actions;
	vector<function<void()>> on_enter_actions = {};
	vector<function<void()>> on_exit_actions = {};
	vector<uint16_t> state_orders_ids = {};

	void enter();
	void exit();
	template<class TimeUnit>
	TimedAction* add_new_timed_action(function<void()> action, chrono::duration<int64_t, TimeUnit> period, AlarmType precision_type);
	template<class TimeUnit>
	TimedAction* register_new_timed_action(function<void()> action, chrono::duration<int64_t, TimeUnit> period, AlarmType precision_type);
	void unregister_all_timed_actions();
	void register_all_timed_actions();
	void unregister_timed_action(TimedAction* timed_action);
	void erase_timed_action(TimedAction* timed_action);
	void add_state_order(uint16_t id);
};

template<class TimeUnit>
TimedAction* State::register_new_timed_action(function<void()> action, chrono::duration<int64_t, TimeUnit> period, AlarmType precision_type){
	TimedAction timed_action = {};
	timed_action.alarm_precision = precision_type;
	timed_action.action = action;
	uint32_t miliseconds = chrono::duration_cast<chrono::milliseconds>(period).count();
	uint32_t microseconds = chrono::duration_cast<chrono::microseconds>(period).count();
	switch (precision_type) {
		case LOW_PRECISION:
			timed_action.period = miliseconds;
			timed_action.id = Time::register_low_precision_alarm(miliseconds, action);
			break;
		case MID_PRECISION:
			timed_action.period = microseconds;
			timed_action.id = Time::register_mid_precision_alarm(microseconds, action);
			break;
		case HIGH_PRECISION:
			timed_action.period = microseconds;
			timed_action.id = Time::register_high_precision_alarm(microseconds, action);
			break;
		default:
			ErrorHandler("Alarm Precision Type does not exist, AlarmType: %d", precision_type);
			break;
	}
	timed_action.is_on = true;
	cyclic_actions.push_back(timed_action);
	return &cyclic_actions.back();
}

template<class TimeUnit>
TimedAction* State::add_new_timed_action(function<void()> action, chrono::duration<int64_t, TimeUnit> period, AlarmType precision_type){
	TimedAction timed_action = {};
	timed_action.alarm_precision = precision_type;
	timed_action.action = action;
	uint32_t miliseconds = chrono::duration_cast<chrono::milliseconds>(period).count();
	uint32_t microseconds = chrono::duration_cast<chrono::microseconds>(period).count();
	switch (precision_type) {
		case LOW_PRECISION:
			timed_action.period = miliseconds;
			break;
		case MID_PRECISION:
			timed_action.period = microseconds;
			break;
		case HIGH_PRECISION:
			timed_action.period = microseconds;
			break;
		default:
			ErrorHandler("Alarm Precision Type does not exist, AlarmType: %d", precision_type);
			return nullptr;
			break;
	}
	cyclic_actions.push_back(timed_action);
	return &cyclic_actions.back();
}


class StateMachine {
public:
	typedef uint8_t state_id;

	state_id initial_state = 0;
	state_id current_state = 0;

	bool is_on = true;


	StateMachine();
	StateMachine(state_id initial_state);

	void add_state(state_id state);
	void add_transition(state_id old_state, state_id new_state, function<bool()> transition);

	template<class TimeUnit>
	TimedAction* add_low_precision_cyclic_action(function<void()> action, chrono::duration<int64_t, TimeUnit> period, state_id state);
	template<class TimeUnit>
	TimedAction* add_low_precision_cyclic_action(function<void()> action, chrono::duration<int64_t, TimeUnit> period, vector<state_id> states);
	template<class TimeUnit>
	TimedAction* add_low_precision_cyclic_action(function<void()> action, chrono::duration<int64_t, TimeUnit> period);

	template<class TimeUnit>
	TimedAction* add_mid_precision_cyclic_action(function<void()> action, chrono::duration<int64_t, TimeUnit> period, state_id state);
	template<class TimeUnit>
	TimedAction* add_mid_precision_cyclic_action(function<void()> action, chrono::duration<int64_t, TimeUnit> period, vector<state_id> states);
	template<class TimeUnit>
	TimedAction* add_mid_precision_cyclic_action(function<void()> action, chrono::duration<int64_t, TimeUnit> period);

	template<class TimeUnit>
	TimedAction* add_high_precision_cyclic_action(function<void()> action, chrono::duration<int64_t, TimeUnit> period);
	template<class TimeUnit>
	TimedAction* add_high_precision_cyclic_action(function<void()> action, chrono::duration<int64_t, TimeUnit> period, state_id state);
	template<class TimeUnit>
	TimedAction* add_high_precision_cyclic_action(function<void()> action, chrono::duration<int64_t, TimeUnit> period, vector<state_id> states);

	void remove_cyclic_action(TimedAction* timed_action);
	void remove_cyclic_action(TimedAction* timed_action, state_id state);

	void add_enter_action(function<void()> action);
	void add_enter_action(function<void()> action, state_id state);

	void add_exit_action(function<void()> action);
	void add_exit_action(function<void()> action, state_id state);

	void force_change_state(state_id new_state);
	void check_transitions();

	void add_state_machine(StateMachine& state_machine, state_id state);

	void refresh_state_orders();

	unordered_map<state_id, State>& get_states();
	
	#ifdef SIM_ON
	uint8_t get_id_in_shm();
	#endif


private:
	unordered_map<state_id, State> states;
	unordered_map<state_id, unordered_map<state_id, function<bool()>>> transitions;
	unordered_map<state_id, vector<StateMachine*>> nested_state_machine;
	

	void enter_state(state_id new_state);
	void exit_state(state_id old_state);
	void register_all_timed_actions(state_id state);
	void unregister_all_timed_actions(state_id state);
	#ifdef SIM_ON
		uint8_t state_machine_id_in_shm;
	#endif
};

template<class TimeUnit>
TimedAction* StateMachine::add_low_precision_cyclic_action(function<void()> action, chrono::duration<int64_t, TimeUnit> period, state_id state) {
	if (not states.contains(state)) {
		ErrorHandler("The state %d is not added to the state machine", state);
		return nullptr;
	}

	uint32_t microseconds = (uint32_t)chrono::duration_cast<chrono::microseconds>(period).count();

	if(microseconds % 1000 != 0){
		ErrorHandler("Low precision cyclic action does not have enough resolution for the desired period, Desired period: %d uS", microseconds);
		return nullptr;
	}
	if(state == current_state && is_on) return states[state].register_new_timed_action(action, period, LOW_PRECISION);
	else return states[state].add_new_timed_action(action, period,LOW_PRECISION);

}

template<class TimeUnit>
TimedAction* StateMachine::add_low_precision_cyclic_action(function<void()> action, chrono::duration<int64_t, TimeUnit> period, vector<state_id> states) {
	TimedAction* timed_action = nullptr;
	for (state_id state : states) {
		timed_action = add_low_precision_cyclic_action(action, period, state);
	}
	return timed_action;
}

template<class TimeUnit>
TimedAction* StateMachine::add_low_precision_cyclic_action(function<void()> action, chrono::duration<int64_t, TimeUnit> period) {
	return add_low_precision_cyclic_action(action, period, current_state);
}

template<class TimeUnit>
TimedAction* StateMachine::add_mid_precision_cyclic_action(function<void()> action, chrono::duration<int64_t, TimeUnit> period, state_id state) {
	if (not states.contains(state)) {
		ErrorHandler("The state %d is not added to the state machine", state);
		return nullptr;
	}

	uint32_t microseconds = (uint32_t)chrono::duration_cast<chrono::microseconds>(period).count();

	if(microseconds % 50 != 0){
		ErrorHandler("Mid precision cyclic action does not have enough resolution for the desired period, Desired period: %d uS", microseconds);
		return nullptr;
	}

	if(state == current_state && is_on) return states[state].register_new_timed_action(action, period, MID_PRECISION);
	else return states[state].add_new_timed_action(action, period,MID_PRECISION);
}

template<class TimeUnit>
TimedAction* StateMachine::add_mid_precision_cyclic_action(function<void()> action, chrono::duration<int64_t, TimeUnit> period, vector<state_id> states) {
	TimedAction* timed_action = nullptr;
	for (state_id state : states) {
		timed_action = add_mid_precision_cyclic_action(action, period, state);
	}
	return timed_action;
}

template<class TimeUnit>
TimedAction* StateMachine::add_mid_precision_cyclic_action(function<void()> action, chrono::duration<int64_t, TimeUnit> period) {
	return add_mid_precision_cyclic_action(action, period, current_state);
}

template<class TimeUnit>
TimedAction* StateMachine::add_high_precision_cyclic_action(function<void()> action, chrono::duration<int64_t, TimeUnit> period, state_id state) {
	if (not states.contains(state)) {
		ErrorHandler("The state %d is not added to the state machine", state);
		return nullptr;
	}

	uint32_t microseconds = (uint32_t)chrono::duration_cast<chrono::microseconds>(period).count();

	if(microseconds < 1){
		ErrorHandler("High precision cyclic action does not have enough resolution for the desired period, Desired period: %d uS", microseconds);
		return nullptr;
	}

	if(state == current_state && is_on) return states[state].register_new_timed_action(action, period, HIGH_PRECISION);
	else return states[state].add_new_timed_action(action, period,HIGH_PRECISION);
}

template<class TimeUnit>
TimedAction* StateMachine::add_high_precision_cyclic_action(function<void()> action, chrono::duration<int64_t, TimeUnit> period, vector<state_id> states) {
	TimedAction* timed_action = nullptr;
	for (state_id state : states) {
		timed_action = add_high_precision_cyclic_action(action, period, state);
	}
	return timed_action;
}

template<class TimeUnit>
TimedAction* StateMachine::add_high_precision_cyclic_action(function<void()> action, chrono::duration<int64_t, TimeUnit> period) {
	return add_high_precision_cyclic_action(action, period, current_state);
}

