#pragma once
#include "C++Utilities/CppUtils.hpp"
#include "ErrorHandler/ErrorHandler.hpp"
#include "HALAL/HALAL.hpp"
#include <array>
#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <unordered_map>

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

template <typename T, size_t Capacity>
class StaticVector {
    std::array<T, Capacity> data;
    size_t size_ = 0;

public:
    constexpr void push_back(const T& value) {
        if (size_ >= Capacity) {
            throw std::out_of_range("StaticVector capacity exceeded");
        }        
        data[size_] = value;
        size_++;
    }

    constexpr auto begin() { return data.begin(); }
    constexpr auto end() { return data.begin() + size_; } 
    
    constexpr const std::array<T, Capacity>& get_array() const { return data; }
    constexpr const size_t size() const { return size_; }
    constexpr  T& operator[](size_t i) { return data[i]; }
    constexpr bool contains(const T& value) const {
        for (size_t i = 0; i < size_; ++i) {
            if (data[i] == value) {
                return true;
            }
        }
        return false;
    }
};
template <typename T, size_t Capacity>
using FixedVector = StaticVector<T, Capacity>;

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
  FixedVector<TimedAction,NUMBER_OF_ACTIONS> cyclic_actions;
  FixedVector<Callback,NUMBER_OF_ACTIONS> on_enter_actions = {};
  FixedVector<Callback,NUMBER_OF_ACTIONS> on_exit_actions = {};
  [[no_unique_address]]FixedVector<uint16_t,Number_of_state_orders> state_orders_ids = {};
  StateEnum state;
  FixedVector<Transition<StateEnum>, NTransitions> transitions;
  public:
  template <typename... T>
        requires are_transitions<StateEnum, T...>
    consteval State(StateEnum state, T... transitions)
        : state(state) {
            (this->transitions.push_back(transitions), ...);
        }
  

  consteval const StateEnum& get_state() const { return state; };
  consteval const auto& get_transitions() const { return transitions; };
  consteval const auto& get_cyclic_actions() const {return cyclic_actions;};
  consteval const auto& get_enter_actions() const {return on_enter_actions;};
  consteval const auto& get_exit_actions() const {return on_exit_actions;};

  consteval void add_cyclic_action(TimedAction *timed_action); 

  consteval void add_enter_action(Callback action){
    on_enter_actions.push_back(action);
  }

  consteval void add_exit_action(Callback action){
    on_exit_actions.push_back(action);
  }

  void enter(){
    for (const auto& action : on_enter_actions) {
          if(action) action();
      }
      register_all_timed_actions();
  }
  void exit(){
    unregister_all_timed_actions();
    for (const auto& action : on_exit_actions) {
        if(action) action();
    }
  }

  void unregister_all_timed_actions(){
    for(size_t i = 0; i < cyclic_actions.size(); i++)
    {
      TimedAction& timed_action = cyclic_actions[i];
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
  }

  void register_all_timed_actions(){
    for(size_t i = 0; i < cyclic_actions.size(); i++)
    {
      TimedAction& timed_action = cyclic_actions[i];
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
        return;
        break;
      }
      timed_action.is_on = true;
    }
  }

  void remove_cyclic_action(TimedAction *timed_action){
    if(timed_action->is_on) unregister_timed_action(timed_action);
    for(size_t i = 0; i < cyclic_actions.size(); i++)
    {
      TimedAction& slot = cyclic_actions[i];
      if(&slot == timed_action){
          slot = TimedAction{}; 
          return;
      }
    }
  }

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
  }

  consteval void add_state_order(uint16_t id){
    state_orders_ids.push_back(id);
  }

  template <class TimeUnit>
  consteval TimedAction * 
  add_low_precision_cyclic_action(Callback action,
                                  chrono::duration<int64_t, TimeUnit> period){
    TimedAction timed_action = {};
    timed_action.alarm_precision = LOW_PRECISION;
    timed_action.action = action;
    constexpr uint32_t miliseconds = chrono::duration_cast<chrono::milliseconds>(period).count();
    timed_action.period = miliseconds;
    
    cyclic_actions.push_back(timed_action);
    return &cyclic_actions[cyclic_actions.size() - 1];
  }

  template <class TimeUnit>
  consteval TimedAction *
  add_mid_precision_cyclic_action(Callback action,
                                  chrono::duration<int64_t, TimeUnit> period){
    TimedAction timed_action = {};
    timed_action.alarm_precision = MID_PRECISION;
    timed_action.action = action;
    constexpr uint32_t microseconds = chrono::duration_cast<chrono::microseconds>(period).count();
    timed_action.period = microseconds;
    
    cyclic_actions.push_back(timed_action);
    return &cyclic_actions[cyclic_actions.size() - 1];
  }

  template <class TimeUnit>
  consteval TimedAction *
  add_high_precision_cyclic_action(Callback action,
                                   chrono::duration<int64_t, TimeUnit> period){
    TimedAction timed_action = {};
    timed_action.alarm_precision = HIGH_PRECISION;
    timed_action.action = action;
    constexpr uint32_t microseconds = chrono::duration_cast<chrono::microseconds>(period).count();
    timed_action.period = microseconds;
    
    cyclic_actions.push_back(timed_action);
    return &cyclic_actions[cyclic_actions.size() - 1];
  }

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
  using Transitions = FixedVector<Transition<StateEnum>, NTransitions>;
  using TAssocs = std::array<std::pair<size_t, size_t>, NStates>;

  StateEnum current_state;
  Transitions transitions;
  TAssocs transitions_assoc;
  std::array<State<StateEnum, NTransitions>, NStates> states;
  unordered_map<StateEnum, StateMachine*> nested_state_machine; 

  consteval void process_state(auto state, size_t offset) {
        for (const auto& t : state.get_transitions()) {
            transitions.push_back(t);
            offset++;
        }

        transitions_assoc[static_cast<size_t>(state.get_state())] = {
            offset - state.get_transitions().size(),
            state.get_transitions().size()};
    }

  inline void enter() { 
      auto& state = states[static_cast<size_t>(current_state)];
      state.enter();
      
  }

  inline void exit() {
      auto& state = states[static_cast<size_t>(current_state)];
      state.exit();
  }

public:

  template <typename... S>
        requires are_states<StateEnum, S...>
    consteval StateMachine(StateEnum initial_state, S... states) {
        current_state = initial_state;
        for(const auto& state : {states...}) {
            this->states[static_cast<size_t>(state.get_state())] = state;
        }
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
                remove_state_orders();
                current_state = t.target;
                enter();
                refresh_state_orders();
                break;
            }
        }
        if (nested_state_machine.contains(current_state)) {
          nested_state_machine[current_state]->check_transitions();
        }
    }

    void force_change_state(StateEnum new_state) {
        exit();
        current_state = new_state;
        enter();
    }
  
  template <class TimeUnit>
  consteval TimedAction *
  add_low_precision_cyclic_action(Callback action,
                                   chrono::duration<int64_t, TimeUnit> period,StateEnum state){
    if (not states.contains(state)) {
      throw "Error: The state is not added to the state machine";
      return nullptr;
    }
    return states[static_cast<size_t>(state)].add_low_precision_cyclic_action(action, period);
  }

  template <class TimeUnit>
  consteval TimedAction *
  add_mid_precision_cyclic_action(Callback action,
                                   chrono::duration<int64_t, TimeUnit> period,StateEnum state){
    if (not states.contains(state)) {
      throw "Error: The state is not added to the state machine";
      return nullptr;
    }
    return states[static_cast<size_t>(state)].add_mid_precision_cyclic_action(action, period);
  }

  template <class TimeUnit>
  consteval TimedAction *
  add_high_precision_cyclic_action(Callback action,
                                   chrono::duration<int64_t, TimeUnit> period,StateEnum state){
    if (not states.contains(state)) {
      throw "Error: The state is not added to the state machine";
      return nullptr;
    }
    return states[static_cast<size_t>(state)].add_high_precision_cyclic_action(action, period);
  }

  constexpr void remove_cyclic_action(TimedAction *timed_action, StateEnum state){
    if (not states.contains(state)) {
      throw "Error: The state is not added to the state machine";
      return;
    }
    states[static_cast<size_t>(state)].remove_cyclic_action(timed_action);
  }

  consteval void add_enter_action(Callback action, StateEnum state){
    if (not states.contains(state)) {
      throw "Error: The state is not added to the state machine";
      return;
    }
    states[state].on_enter_actions.push_back(action);
  }

  consteval void add_exit_action(Callback action, StateEnum state){
    if (not states.contains(state)) {
      throw "Error: The state is not added to the state machine";
      return;
    }
    states[state].on_exit_actions.push_back(action);
  }

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


  inline void refresh_state_orders(){
  #ifdef STLIB_ETH
    if(states[current_state].state_orders_ids.size() != 0) StateOrder::add_state_orders(states[current_state].state_orders_ids); 
  #endif
  }

  inline void remove_state_orders(){
  #ifdef STLIB_ETH
    if(states[current_state].state_orders_ids.size() != 0) StateOrder::remove_state_orders(states[current_state].state_orders_ids);
  #endif
  }

  consteval std::array<State<StateEnum, NTransitions>, NStates>& get_states(){
    return states;
  }


  template <typename EnumType, typename... Transitions>
  requires are_transitions<StateEnum, Transitions...>
  static consteval auto make_state(StateEnum state, Callback action,
                                  Transitions... transitions) {
    constexpr size_t number_of_transitions = sizeof...(transitions);
    return State<StateEnum, number_of_transitions>(state, action,
                                          transitions...);
  }

  template <typename EnumType, typename... States>
    requires are_states<StateEnum, States...>
  static consteval auto make_state_machine(StateEnum initial_state, States... states) {
    constexpr size_t number_of_states = sizeof...(states);
    constexpr size_t number_of_transitions = (states.get_transitions().size() + ... + 0);

    return StateMachine<StateEnum, number_of_states, number_of_transitions>(initial_state, states...);
  }



#ifdef SIM_ON
  uint8_t get_id_in_shm();
  uint8_t state_machine_id_in_shm;
#endif
};

