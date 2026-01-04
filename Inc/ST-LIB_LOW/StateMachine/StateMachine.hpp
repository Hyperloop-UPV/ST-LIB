/*
 * Created by Alejandro
 */

#pragma once
#include "C++Utilities/CppUtils.hpp"
#include "ErrorHandler/ErrorHandler.hpp"
#include "HALAL/HALAL.hpp"
#include <array>
#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

#ifdef STLIB_ETH
#include "StateMachine/StateOrder.hpp"
#endif
#ifdef SIM_ON
#include "HALALMock/Services/SharedMemory/SharedMemory.hpp"
#endif

using ms = std::chrono::milliseconds;
using us = std::chrono::microseconds;
using s = std::chrono::seconds;

using Callback = void (*)();
using Guard = bool (*)();

static constexpr size_t NUMBER_OF_ACTIONS = 20;

enum AlarmType { LOW_PRECISION, MID_PRECISION, HIGH_PRECISION };

class TimedAction {
public:
  Callback action = nullptr;
  uint32_t period = 0;
  AlarmType alarm_precision = LOW_PRECISION;
  uint8_t id = 255;
  bool is_on = false;

  TimedAction() = default;
};

template <class StateEnum>
struct Transition {
    StateEnum target;
    Guard predicate;
};

template <class StateEnum, typename... T>
concept are_transitions = (std::same_as<T, Transition<StateEnum>> && ...);

template <class StateEnum, size_t NTransitions,size_t Number_of_state_orders=0>
class State {
private:
  std::array<TimedAction,NUMBER_OF_ACTIONS> cyclic_actions;
  std::array<Callback,NUMBER_OF_ACTIONS> on_enter_actions = {};
  std::array<Callback,NUMBER_OF_ACTIONS> on_exit_actions = {};
  [[no_unique_address]]std::array<uint16_t,Number_of_state_orders> state_orders_ids = {};
  StateEnum state;
  std::array<Transition<StateEnum>, NTransitions> transitions;
  public:
  template <typename... T>
        requires are_transitions<StateEnum, T...>
    consteval State(StateEnum state, T... transitions)
        : state(state), transitions({transitions...}) {}
  

  consteval const StateEnum& get_state() const { return state; };
  consteval const auto& get_transitions() const { return transitions; };
  consteval const auto& get_cyclic_actions() const {return cyclic_actions;};
  consteval const auto& get_enter_actions() const {return on_enter_actions;};
  consteval const auto& get_exit_actions() const {return on_exit_actions;};

  consteval void add_cyclic_action(TimedAction *timed_action); 

  consteval void add_enter_action(Callback action){
    for(auto& slot : on_enter_actions){
        if(slot == nullptr){
            slot = action;
            return;
        }
    }
  };

  consteval void add_exit_action(Callback action){
    for(auto& slot : on_exit_actions){
        if(slot == nullptr){
            slot = action;
            return;
        }
    }
  };

  void enter(){
    for (const auto& action : on_enter_actions) {
          if(action) action();
      }
      register_all_timed_actions();
  };
  void exit(){
    unregister_all_timed_actions();
    for (const auto& action : on_exit_actions) {
        if(action) action();
    }
  };

  void unregister_all_timed_actions(){
    for(TimedAction& timed_action : cyclic_actions){
      if(timed_action.action == nullptr) continue;
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
  }; 

  void register_all_timed_actions(){
    for(TimedAction& timed_action : cyclic_actions){
      if(timed_action.action == nullptr) continue;
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
  };

  void remove_cyclic_action(TimedAction *timed_action){
    if(timed_action->is_on) unregister_timed_action(timed_action);
    for(auto& slot : cyclic_actions){
      if(&slot == timed_action){
          slot = TimedAction{};
          return;
      }
    }
  };

  void unregister_timed_action(TimedAction* timed_action){
    switch(timed_action->alarm_precision){
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
      ErrorHandler("Cannot unregister timed action with erroneus alarm precision, Alarm Precision Type: %d", timed_action->alarm_precision);
      return;
      break;
    }
    timed_action->is_on = false;
  };

  void add_state_order(uint16_t id){
    state_orders_ids.push_back(id);
  };

  template <class TimeUnit>
  consteval TimedAction * 
  add_low_precision_cyclic_action(Callback action,
                                  chrono::duration<int64_t, TimeUnit> period){
  TimedAction timed_action = {};
  timed_action.alarm_precision = LOW_PRECISION;
  timed_action.action = action;
  uint32_t miliseconds = chrono::duration_cast<chrono::milliseconds>(period).count();
  timed_action.period = miliseconds;
  
  for(auto& slot : cyclic_actions){
      if(slot.action == nullptr){
          slot = timed_action;
          return &slot;
      }
  }
  return nullptr;
  };

  template <class TimeUnit>
  consteval TimedAction *
  add_mid_precision_cyclic_action(Callback action,
                                  chrono::duration<int64_t, TimeUnit> period){
  TimedAction timed_action = {};
  timed_action.alarm_precision = MID_PRECISION;
  timed_action.action = action;
  uint32_t microseconds = chrono::duration_cast<chrono::microseconds>(period).count();
  timed_action.period = microseconds;
  
  for(auto& slot : cyclic_actions){
      if(slot.action == nullptr){
          slot = timed_action;
          return &slot;
      }
  }
  return nullptr;
  };

  template <class TimeUnit>
  consteval TimedAction *
  add_high_precision_cyclic_action(Callback action,
                                   chrono::duration<int64_t, TimeUnit> period){
  TimedAction timed_action = {};
  timed_action.alarm_precision = HIGH_PRECISION;
  timed_action.action = action;
  uint32_t microseconds = chrono::duration_cast<chrono::microseconds>(period).count();
  timed_action.period = microseconds;
  
  for(auto& slot : cyclic_actions){
      if(slot.action == nullptr){
          slot = timed_action;
          return &slot;
      }
  }
  return nullptr;
  };

};


template <typename T, class StateEnum>
struct is_state : std::false_type {};

template <class StateEnum, size_t N>
struct is_state<State<StateEnum, N>, StateEnum> : std::true_type {}; 

template <class StateEnum, typename... Ts>
concept are_states = (is_state<Ts, StateEnum>::value && ...);

template <class StateEnum, size_t NStates, size_t NTransitions>
class StateMachine {
private:
  using Transitions = std::array<Transition<StateEnum>, NTransitions>;
  using TAssocs = std::array<std::pair<size_t, size_t>, NStates>;

  StateEnum current_state;
  Transitions transitions;
  TAssocs transitions_assoc;
  std::array<State<StateEnum, NTransitions>, NStates> states;
  unordered_map<StateEnum, StateMachine*> nested_state_machine; //ya veremos que hacemos con esto



  consteval void process_state(auto state, size_t offset) {
        for (const auto& t : state.get_transitions()) {
            transitions[offset++] = t;
        }

        transitions_assoc[static_cast<size_t>(state.get_state())] = {
            offset - state.get_transitions().size(),
            state.get_transitions().size()};
    }

  void enter() { 
      auto& state = states[static_cast<size_t>(current_state)];
      state.enter();
      
  }

  void exit() {
      auto& state = states[static_cast<size_t>(current_state)];
      state.exit();
  }

public:

  template <typename... S>
        requires are_states<StateEnum, S...>
    consteval StateMachine(StateEnum initial_state, S... states) {
        current_state = initial_state;
        size_t offset = 0;
        ((process_state(states, offset),
          offset += states.get_transitions().size()),
         ...);
    }

    

    void check_transitions() {
        auto& [i, n] = transitions_assoc[static_cast<size_t>(current_state)];
        for (auto index = i; index < i + n; ++index) {
            const auto& t = transitions[index];
            if (t.predicate()) {
                exit();
                current_state = t.target;
                enter();
                break;
            }
        }
        if (nested_state_machine.contains(current_state)) {
          nested_state_machine[current_state]->check_transitions();
        }
    };

    void force_change_state(StateEnum new_state) {
        exit();
        current_state = new_state;
        enter();
    };
  
    //Falta helper crear transiciones y crear estado
  consteval void add_cyclic_action(TimedAction *timed_action, StateEnum state); //helpers que llamarán en tiempo de compilacion a las funciones de state

  void remove_cyclic_action(TimedAction *timed_action, StateEnum state);

  consteval void add_enter_action(Callback action, StateEnum state){
    if (not states.contains(state)) {
      throw "Error: The state is not added to the state machine";
      return;
    }
    states[state].on_enter_actions.push_back(action);
  };

  consteval void add_exit_action(Callback action, StateEnum state){
    if (not states.contains(state)) {
      throw "Error: The state is not added to the state machine";
      return;
    }
    states[state].on_exit_actions.push_back(action);
  };

  void add_state_machine(StateMachine& state_machine, uint8_t state) {
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

  void add_state_machine(StateMachine &state_machine, StateEnum state);

  void refresh_state_orders(){
  #ifdef STLIB_ETH
    if(states[current_state].state_orders_ids.size() != 0) StateOrder::add_state_orders(states[current_state].state_orders_ids); //Por ver
  #endif
  }

  consteval std::array<State<StateEnum, NTransitions>, NStates> states& get_states(){
    return states;
  }

  template <typename StateEnum, typename... Transitions>
    requires are_transitions<StateEnum, Transitions...>
  static consteval auto make_state(StateEnum state, Callback action,
                                  Transitions... transitions) {
    constexpr size_t NTransitions = sizeof...(transitions);
    return State<StateEnum, NTransitions>(state, action,
                                          transitions...);
  }



#ifdef SIM_ON
  uint8_t get_id_in_shm();
  uint8_t state_machine_id_in_shm;
#endif
};

