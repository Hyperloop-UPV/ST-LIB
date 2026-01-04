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
  Callback action;
  uint32_t period;
  AlarmType alarm_precision;
  uint8_t id = -1;

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
  consteval const auto& get_cyclic_actions() const {return cyclic_actions};
  consteval const auto& get_enter_actions() const {return on_enter_actions};
  consteval const auto& get_exit_actions() const {return on_exit_actions};

  template <class TimeUnit>
  consteval TimedAction *
  add_low_precision_cyclic_action(Callback action,
                                  chrono::duration<int64_t, TimeUnit> period);
  template <class TimeUnit>
  consteval TimedAction *
  add_mid_precision_cyclic_action(Callback action,
                                  chrono::duration<int64_t, TimeUnit> period);

  template <class TimeUnit>
  consteval TimedAction *
  add_high_precision_cyclic_action(Callback action,
                                   chrono::duration<int64_t, TimeUnit> period);

  void enter();
  void exit();

  void unregister_all_timed_actions(); 
  void register_all_timed_actions();
  void erase_timed_action(TimedAction* timed_action);
  void add_state_order(uint16_t id);
};


template <class TimeUnit>
consteval TimedAction * State::add_low_precision_cyclic_action(Callback action,
                            chrono::duration<int64_t, TimeUnit> period) {
  TimedAction timed_action = {};
  timed_action.alarm_precision = LOW_PRECISION;
  timed_action.action = action;
  uint32_t miliseconds = chrono::duration_cast<chrono::milliseconds>(period).count();
  timed_action.period = miliseconds;
  cyclic_actions.push_back(timed_action);
  return &cyclic_actions.back();
}

template <class TimeUnit>
consteval TimedAction * State::add_mid_precision_cyclic_action(Callback action,
                            chrono::duration<int64_t, TimeUnit> period) {
  TimedAction timed_action = {};
  timed_action.alarm_precision = MID_PRECISION;
  timed_action.action = action;
  uint32_t microseconds = chrono::duration_cast<chrono::microseconds>(period).count();
  timed_action.period = microseconds;
  cyclic_actions.push_back(timed_action);
  return &cyclic_actions.back();
}

template <class TimeUnit>
consteval TimedAction * State::add_high_precision_cyclic_action(Callback action,
                            chrono::duration<int64_t, TimeUnit> period) {
  TimedAction timed_action = {};
  timed_action.alarm_precision = HIGH_PRECISION;
  timed_action.action = action;
  uint32_t microseconds = chrono::duration_cast<chrono::microseconds>(period).count();
  timed_action.period = microseconds;
  cyclic_actions.push_back(timed_action);
  return &cyclic_actions.back();
}
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


public:
  
  bool is_on = true;

  StateMachine();
  StateMachine(StateEnum initial_state);

  consteval void add_state(StateEnum state){
    states[state] = State<StateEnum,NTransitions,>(state);
  }
  void add_transition(StateEnum old_state, StateEnum new_state,
                      Guard transition);


  void remove_cyclic_action(TimedAction *timed_action);
  void remove_cyclic_action(TimedAction *timed_action, StateEnum state);

  void add_enter_action(Callback action);
  void add_enter_action(Callback action, StateEnum state);

  void add_exit_action(Callback action);
  void add_exit_action(Callback action, StateEnum state);

  void force_change_state(StateEnum new_state);
  void check_transitions();

  void add_state_machine(StateMachine &state_machine, StateEnum state);

  void refresh_state_orders();

  unordered_map<StateEnum, State> &get_states();

#ifdef SIM_ON
  uint8_t get_id_in_shm();
#endif

private:
  unordered_map<StateEnum, State> states;
  unordered_map<StateEnum, unordered_map<StateEnum, Guard>>
      transitions;
  unordered_map<StateEnum, StateMachine *> nested_state_machine;

  void enter_state(StateEnum new_state);
  void exit_state(StateEnum old_state);
  void register_all_timed_actions(StateEnum state);
  void unregister_all_timed_actions(StateEnum state);
#ifdef SIM_ON
  uint8_t state_machine_id_in_shm;
#endif
};

template <class TimeUnit>
TimedAction *StateMachine::add_low_precision_cyclic_action(
    Callback action, chrono::duration<int64_t, TimeUnit> period,
    StateEnum state) {
  if (not states.contains(state)) {
    ErrorHandler("The state %d is not added to the state machine", state);
    return nullptr;
  }

  uint32_t microseconds =
      (uint32_t)chrono::duration_cast<chrono::microseconds>(period).count();

  if (microseconds % 1000 != 0) {
    ErrorHandler("Low precision cyclic action does not have enough resolution "
                 "for the desired period, Desired period: %d uS",
                 microseconds);
    return nullptr;
  }
  if (state == current_state && is_on)
    return states[state].register_new_timed_action(action, period,
                                                   LOW_PRECISION);
  else
    return states[state].add_new_timed_action(action, period, LOW_PRECISION);
}

template <class TimeUnit>
TimedAction *StateMachine::add_low_precision_cyclic_action(
    Callback action, chrono::duration<int64_t, TimeUnit> period,
    vector<StateEnum> states) {
  TimedAction *timed_action = nullptr;
  for (StateEnum state : states) {
    timed_action = add_low_precision_cyclic_action(action, period, state);
  }
  return timed_action;
}

template <class TimeUnit>
TimedAction *StateMachine::add_low_precision_cyclic_action(
    Callback action, chrono::duration<int64_t, TimeUnit> period) {
  return add_low_precision_cyclic_action(action, period, current_state);
}

template <class TimeUnit>
TimedAction *StateMachine::add_mid_precision_cyclic_action(
    Callback action, chrono::duration<int64_t, TimeUnit> period,
    StateEnum state) {
  if (not states.contains(state)) {
    ErrorHandler("The state %d is not added to the state machine", state);
    return nullptr;
  }

  uint32_t microseconds =
      (uint32_t)chrono::duration_cast<chrono::microseconds>(period).count();

  if (microseconds % 50 != 0) {
    ErrorHandler("Mid precision cyclic action does not have enough resolution "
                 "for the desired period, Desired period: %d uS",
                 microseconds);
    return nullptr;
  }

  if (state == current_state && is_on)
    return states[state].register_new_timed_action(action, period,
                                                   MID_PRECISION);
  else
    return states[state].add_new_timed_action(action, period, MID_PRECISION);
}

template <class TimeUnit>
TimedAction *StateMachine::add_mid_precision_cyclic_action(
    Callback action, chrono::duration<int64_t, TimeUnit> period,
    vector<StateEnum> states) {
  TimedAction *timed_action = nullptr;
  for (StateEnum state : states) {
    timed_action = add_mid_precision_cyclic_action(action, period, state);
  }
  return timed_action;
}

template <class TimeUnit>
TimedAction *StateMachine::add_mid_precision_cyclic_action(
    Callback action, chrono::duration<int64_t, TimeUnit> period) {
  return add_mid_precision_cyclic_action(action, period, current_state);
}

template <class TimeUnit>
TimedAction *StateMachine::add_high_precision_cyclic_action(
    Callback action, chrono::duration<int64_t, TimeUnit> period,
    StateEnum state) {
  if (not states.contains(state)) {
    ErrorHandler("The state %d is not added to the state machine", state);
    return nullptr;
  }

  uint32_t microseconds =
      (uint32_t)chrono::duration_cast<chrono::microseconds>(period).count();

  if (microseconds < 1) {
    ErrorHandler("High precision cyclic action does not have enough resolution "
                 "for the desired period, Desired period: %d uS",
                 microseconds);
    return nullptr;
  }

  if (state == current_state && is_on)
    return states[state].register_new_timed_action(action, period,
                                                   HIGH_PRECISION);
  else
    return states[state].add_new_timed_action(action, period, HIGH_PRECISION);
}

template <class TimeUnit>
TimedAction *StateMachine::add_high_precision_cyclic_action(
    Callback action, chrono::duration<int64_t, TimeUnit> period,
    vector<StateEnum> states) {
  TimedAction *timed_action = nullptr;
  for (StateEnum state : states) {
    timed_action = add_high_precision_cyclic_action(action, period, state);
  }
  return timed_action;
}

template <class TimeUnit>
TimedAction *StateMachine::add_high_precision_cyclic_action(
    Callback action, chrono::duration<int64_t, TimeUnit> period) {
  return add_high_precision_cyclic_action(action, period, current_state);
}
