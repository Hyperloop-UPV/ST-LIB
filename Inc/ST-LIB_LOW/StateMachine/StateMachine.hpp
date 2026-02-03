#pragma once
#include "C++Utilities/CppUtils.hpp"
#include "C++Utilities/StaticVector.hpp"
#include "ErrorHandler/ErrorHandler.hpp"
#include "HALAL/HALAL.hpp"
#include <array>
#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <unordered_map>
#include <algorithm>

#ifdef STLIB_ETH
#include "StateMachine/StateOrder.hpp"
#endif
#ifdef SIM_ON
#include "HALALMock/Services/SharedMemory/SharedMemory.hpp"
#endif

using ms = std::chrono::milliseconds;
using us = std::chrono::microseconds;
using s = std::chrono::seconds;

template <typename T, size_t Capacity>
using FixedVector = StaticVector<T, Capacity>;

template<class StateEnum>
concept IsEnum = std::is_enum_v<StateEnum>;

template<class T>
concept ValidTime = std::same_as<T, std::chrono::milliseconds> || std::same_as<T, std::chrono::microseconds>;

using Callback = void (*)();
using Guard = bool (*)();


static constexpr size_t NUMBER_OF_ACTIONS = 20;

enum AlarmType { Milliseconds = 0, Microseconds = 1 };

class TimedAction {
public:
  Callback action = nullptr;
  uint32_t period = 0;
  AlarmType alarm_precision = Milliseconds;
  uint8_t id = 255;
  bool is_on = false;

  TimedAction() = default;
  constexpr bool operator==(const TimedAction&) const = default;
};

template <IsEnum StateEnum>
struct Transition {
    StateEnum target;
    Guard predicate;
    constexpr bool operator==(const Transition&) const = default;
};

template <class StateEnum, typename... T>
concept are_transitions = (std::same_as<T, Transition<StateEnum>> && ...);


template <IsEnum StateEnum, size_t NTransitions,size_t Number_of_state_orders=0>
class State { 
private:
  FixedVector<TimedAction,NUMBER_OF_ACTIONS> cyclic_actions = {};
  FixedVector<Callback,NUMBER_OF_ACTIONS> on_enter_actions = {};
  FixedVector<Callback,NUMBER_OF_ACTIONS> on_exit_actions = {};
  StateEnum state = {};
  FixedVector<Transition<StateEnum>, NTransitions> transitions = {};

  public:
  [[no_unique_address]]FixedVector<uint16_t,Number_of_state_orders> state_orders_ids = {};
  static constexpr size_t transition_count = NTransitions;

  template <typename... T>
        requires are_transitions<StateEnum, T...>
    consteval State(StateEnum state, T... transitions)
        : state(state) 
        {
            (this->transitions.push_back(transitions), ...);
        }

  consteval State() = default;

  template <size_t N, size_t O>
  consteval State(const State<StateEnum, N, O>& other){
      state = other.get_state();
      for(const auto& t : other.get_transitions()) transitions.push_back(t);
      for(const auto& a : other.get_cyclic_actions()) cyclic_actions.push_back(a);
      for(const auto& a : other.get_enter_actions()) on_enter_actions.push_back(a);
      for(const auto& a : other.get_exit_actions()) on_exit_actions.push_back(a);
      #ifdef STLIB_ETH
      for(const auto& id : other.state_orders_ids) state_orders_ids.push_back(id);
      #endif
  }

  constexpr bool operator==(const State&) const = default;
    
  

  constexpr const StateEnum& get_state() const { return state; };
  constexpr const auto& get_transitions() const { return transitions; };
  constexpr const auto& get_cyclic_actions() const {return cyclic_actions;};
  constexpr const auto& get_enter_actions() const {return on_enter_actions;};
  constexpr const auto& get_exit_actions() const {return on_exit_actions;};


  consteval void add_enter_action(Callback action)
  {
    on_enter_actions.push_back(action);
  }

  consteval void add_exit_action(Callback action)
  {
    on_exit_actions.push_back(action);
  }

  void enter()
  {
    for (const auto& action : on_enter_actions) 
      {
          if(action) action();
      }
      register_all_timed_actions();
  }

  void exit()
  {
    unregister_all_timed_actions();
    for (const auto& action : on_exit_actions) 
      {
        if(action) action();
      }
  }

  void unregister_all_timed_actions()
  {
    for(size_t i = 0; i < cyclic_actions.size(); i++)
    {
      TimedAction& timed_action = cyclic_actions[i];
      if(timed_action.action == nullptr){continue;}
      if(!timed_action.is_on){ continue;}

      Scheduler::unregister_task(timed_action.id);
      timed_action.is_on = false;
    }
  }

  void register_all_timed_actions()
  {
    for(size_t i = 0; i < cyclic_actions.size(); i++)
    {
      TimedAction& timed_action = cyclic_actions[i];
      if(timed_action.action == nullptr){ continue;}

      switch(timed_action.alarm_precision)
      {
      case Milliseconds:
        timed_action.id = Scheduler::register_task((timed_action.period * 1000), timed_action.action);
        break;
      case Microseconds:
        timed_action.id = Scheduler::register_task(timed_action.period, timed_action.action);
        break;

      default:
        ErrorHandler("Invalid Alarm Precision");
        return;
        break;
      }
      if(timed_action.id == Scheduler::INVALID_ID)
      {
        ErrorHandler("Failed to register timed action");
        return;
      }
      timed_action.is_on = true;
    }
  }

  void remove_cyclic_action(TimedAction *timed_action)
  {
    if(timed_action->is_on){ unregister_timed_action(timed_action);}

    for(size_t i = 0; i < cyclic_actions.size(); i++)
    {
      TimedAction& slot = cyclic_actions[i];
      if(&slot == timed_action)
      {
          slot = TimedAction{}; 
          return;
      }
    }
  }

  void unregister_timed_action(TimedAction* timed_action)
  {
    Scheduler::unregister_task(timed_action->id);
    timed_action->is_on = false;
  }

  constexpr void add_state_order(uint16_t id)
  {
    state_orders_ids.push_back(id);
  }

  template <ValidTime TimeUnit>
  consteval TimedAction * 
  add_cyclic_action(Callback action,
                                  TimeUnit period){
    TimedAction timed_action = {};
    if constexpr (std::is_same_v<TimeUnit, std::chrono::milliseconds>){
        timed_action.alarm_precision = Milliseconds;
        timed_action.period = period.count();
    }

    else if constexpr (std::is_same_v<TimeUnit, std::chrono::microseconds>){
        timed_action.alarm_precision = Microseconds;
        timed_action.period = period.count();
    }

    else {
        ErrorHandler("Invalid Time Unit");
    }
    
    timed_action.action = action;
    cyclic_actions.push_back(timed_action);
    return &cyclic_actions[cyclic_actions.size() - 1];
  }


};


template <typename T, class StateEnum>
struct is_state : std::false_type {};

template <class StateEnum, size_t N>
struct is_state<State<StateEnum, N>, StateEnum> : std::true_type {}; 

template <typename T, class StateEnum>
concept IsState = is_state<T, StateEnum>::value;

template <class StateEnum, typename... Ts>
concept are_states = (IsState<Ts, StateEnum> && ...);

/// Interface for State Machines to allow other classes to interact with the state machine without knowing its implementation
class IStateMachine {
    public:
    virtual constexpr ~IStateMachine() = default;
    virtual void check_transitions() = 0;
    virtual void set_on(bool is_on) = 0;
    virtual void force_change_state(size_t state) = 0;
    virtual size_t get_current_state_id() const = 0;
    constexpr bool operator==(const IStateMachine&) const = default;
};

template <class StateEnum, size_t NStates, size_t NTransitions>
class StateMachine : public IStateMachine {
private:  
  struct NestedPair 
  {
      StateEnum state;
      IStateMachine* machine;
      constexpr bool operator==(const NestedPair&) const = default;
  };

public:
  StateEnum current_state;

  constexpr ~StateMachine() override = default;

  void force_change_state(size_t state) override 
  {
      StateEnum new_state = static_cast<StateEnum>(state);
      if(current_state == new_state)
      { 
        return;
      }
      #ifdef STLIB_ETH
      remove_state_orders();
      #endif
      exit();
      current_state = new_state;
      enter();
      #ifdef STLIB_ETH
      refresh_state_orders();
      #endif
  }

  size_t get_current_state_id() const override
  {
      return static_cast<size_t>(current_state);
  }

  bool is_on = true;
  void set_on(bool is_on) override 
  {
    this->is_on = is_on;
  }

private:
  FixedVector<State<StateEnum, NTransitions>, NStates> states;
  FixedVector<Transition<StateEnum>, NTransitions> transitions = {};
  std::array<std::pair<size_t, size_t>, NStates> transitions_assoc ={};
  FixedVector<NestedPair, NStates> nested_state_machine = {};

  constexpr bool operator==(const StateMachine&) const = default;

  template <typename State>
  consteval void process_state(const State& state, size_t offset) 
  {
        for (const auto& t : state.get_transitions()) 
        {
            transitions.push_back(t);
            offset++;
        }

        transitions_assoc[static_cast<size_t>(state.get_state())] = {
            offset - state.get_transitions().size(),
            state.get_transitions().size()};
  }

  inline void enter() 
  { 
      auto& state = states[static_cast<size_t>(current_state)];
      state.enter();
      
  }

  inline void exit() 
  {
      auto& state = states[static_cast<size_t>(current_state)];
      state.exit();
  }

public:

  template <IsState<StateEnum>... S>
    consteval StateMachine(StateEnum initial_state, S... states) 
    : current_state(initial_state)
     {        
        //Sort states by their enum value
        using StateType = State<StateEnum, NTransitions>;
        std::array<StateType, sizeof...(S)> sorted_states;
        size_t index = 0;
        ((sorted_states[index++] = StateType(states)), ...);
    
        for(size_t i = 0; i < sorted_states.size(); i++){
            for(size_t j = 0; j < sorted_states.size() - 1; j++){
                if(sorted_states[j].get_state() > sorted_states[j+1].get_state()){
                    auto temp = sorted_states[j];
                    sorted_states[j] = sorted_states[j+1];
                    sorted_states[j+1] = temp;
                }
            }
        }
        
        //Check that states are contiguous and start from 0
        for(size_t i = 0; i < sorted_states.size(); i++) {
            if(static_cast<size_t>(sorted_states[i].get_state()) != i){
                ErrorHandler("States Enum must be contiguous and start from 0");
            }
        }

        for(size_t i = 0; i < sorted_states.size(); i++) {
            this->states.push_back(sorted_states[i]);
        }

        size_t offset = 0;
        for(const auto& s : sorted_states) {
             process_state(s, offset);
             offset += s.get_transitions().size();
        }
    }

    

    void check_transitions() override 
    {
        auto& [i, n] = transitions_assoc[static_cast<size_t>(current_state)];
        for (auto index = i; index < i + n; ++index) 
        {
            const auto& t = transitions[index];
            if (t.predicate()) 
            {
                exit();
                #ifdef STLIB_ETH
                remove_state_orders();
                #endif
                current_state = t.target;
                enter();
                #ifdef STLIB_ETH
                refresh_state_orders();
                #endif
                break;
            }
        }

        for(auto& nested : nested_state_machine)
        {
          if(nested.state == current_state){
            nested.machine->check_transitions();
            break;
          }
        }
    }

    void start()
    {
      enter();
    }


    template <size_t N, size_t O>
    void force_change_state(const State<StateEnum, N, O>& state)
    {
      StateEnum new_state = state.get_state();
      if(current_state == new_state)
      {
       return;
      }
        
        exit();
        #ifdef STLIB_ETH
        remove_state_orders();
        #endif
        current_state = new_state;
        enter();
        #ifdef STLIB_ETH
        refresh_state_orders();
        #endif
    }
  


  template <ValidTime TimeUnit, size_t N, size_t O>
  consteval TimedAction * 
  add_cyclic_action(Callback action,
                                  TimeUnit period,const State<StateEnum, N, O>& state)
  {
    for(size_t i = 0; i < states.size(); ++i)
    {
        if(states[i].get_state() == state.get_state())
        {
            return states[i].add_cyclic_action(action, period);
        }
    }
    ErrorHandler("Error: The state is not added to the state machine");
    return nullptr;
  }

  template <size_t N, size_t O>
  void remove_cyclic_action(TimedAction *timed_action, const State<StateEnum, N, O>& state)
  {
    for(size_t i = 0; i < states.size(); ++i)
    {
        if(states[i].get_state() == state.get_state())
        {
           states[i].remove_cyclic_action(timed_action);
           return;
          }
    }
    ErrorHandler("Error: The state is not added to the state machine");
  }

  template <size_t N, size_t O>
  consteval void add_enter_action(Callback action, const State<StateEnum, N, O>& state)
  {
    for(size_t i = 0; i < states.size(); ++i)
    {
        if(states[i].get_state() == state.get_state())
        {
            states[i].add_enter_action(action);
            return;
        }
    }
    ErrorHandler("Error: The state is not added to the state machine");
  }

  template <size_t N, size_t O>
  consteval void add_exit_action(Callback action, const State<StateEnum, N, O>& state)
  {
    for(size_t i = 0; i < states.size(); ++i)
    {
        if(states[i].get_state() == state.get_state())
        {
            states[i].add_exit_action(action);
            return;
        }
    }
    ErrorHandler("Error: The state is not added to the state machine");
  }

  template <size_t N, size_t O>
  constexpr void add_state_machine(IStateMachine& state_machine, const State<StateEnum, N, O>& state)
  {
      for(auto& nested : nested_state_machine)
      {
        if(nested.state == state.get_state())
        {
          ErrorHandler("Only one Nested State Machine can be added per state, tried to add to state: %d", static_cast<int>(state.get_state()));
          return;
        }
      }
      nested_state_machine.push_back({state.get_state(), &state_machine});
  }

  void get_current_state(StateEnum& state) const
  {
      state = current_state;
  }

  void get_current_state(uint8_t& state) const
  {
      state = static_cast<uint8_t>(current_state);
  }


  inline void refresh_state_orders()
  {
  #ifdef STLIB_ETH
    if(states[static_cast<size_t>(current_state)].state_orders_ids.size() != 0) 
    {
        std::span<uint16_t> ids(states[static_cast<size_t>(current_state)].state_orders_ids.get_data(), 
                                states[static_cast<size_t>(current_state)].state_orders_ids.size());
        StateOrder::add_state_orders(ids); 
    }
  #endif
  }

  inline void remove_state_orders()
  {
  #ifdef STLIB_ETH
    if(states[static_cast<size_t>(current_state)].state_orders_ids.size() != 0) 
    {
        std::span<uint16_t> ids(states[static_cast<size_t>(current_state)].state_orders_ids.get_data(), 
                                states[static_cast<size_t>(current_state)].state_orders_ids.size());
        StateOrder::remove_state_orders(ids);
    }
  #endif
  }

  constexpr auto& get_states()
  {
    return states;
  }

#ifdef SIM_ON
  uint8_t get_id_in_shm();
  uint8_t state_machine_id_in_shm;
#endif
};

/* @brief Helper function to create a State instance
 *
 * @tparam StateEnum Enum type representing the states
 * @tparam Transitions Variadic template parameter pack representing the transitions
 * @param state The state enum value
 * @param transitions The transitions associated with the state
 * @return A State instance initialized with the provided state and transitions
*
*/

template <typename StateEnum, typename... Transitions>
  requires are_transitions<StateEnum, Transitions...>
  consteval auto make_state(StateEnum state, Transitions... transitions) {
    constexpr size_t number_of_transitions = sizeof...(transitions);
    return State<StateEnum, number_of_transitions>(state,transitions...);
  }

  /* @brief Helper function to create a StateMachine instance
  *
  * @tparam States Variadic template parameter pack representing the states
  * @param initial_state The initial state enum value
  * @param states The states to be included in the state machine
  * @return A StateMachine instance initialized with the provided initial state and states
  */

  template <typename StateEnum, typename... States>
    requires are_states<StateEnum, States...>
  consteval auto make_state_machine(StateEnum initial_state, States... states) {
    constexpr size_t number_of_states = sizeof...(states);
    constexpr size_t number_of_transitions = (std::remove_reference_t<States>::transition_count + ... + 0);

    return StateMachine<StateEnum, number_of_states, number_of_transitions>(initial_state, states...);
  }
