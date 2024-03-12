#pragma once

#include <optional>
#include <iostream>
#include <tuple>
#include <variant>

template <typename... States>
class state_machine
{
private:
    // the tuple m_states holds all states we'll define
    std::tuple<States...> m_states;
    // in this variant we hold a reference to the current state, it's initialized by the state at index 0
    std::variant<States*...> m_current_state{ &std::get<0>(m_states) };

public:
    // we can change a state by calling set state with a state type
    template <typename State>
    void set_state() {
        m_current_state = &std::get<State>(m_states);
    }

    // we can define certain events which call a dedicated state transition
    template <typename Event>
    void on_state_transition(const Event& event)
    {
        auto execute_on_call = [this, &event] (auto current_state) {
            current_state->on_state_transition(event).execute(*this);
        };
        // std::visit "visits" the current_state and calls the lambda with the current state
        // this means every possible state needs to implement the execute function inside the lambda
        std::visit(execute_on_call, m_current_state);
    }
    
    // we call on_update of each state also with std::visit like above
    void on_update()
    {
        auto execute_on_call = [this] (auto current_state) {
            current_state->on_update();
        };
        std::visit(execute_on_call, m_current_state);
    }
};

// the state transition type, where the template represents the target state
template <typename State>
struct state_transition_to
{
    // on execute we're setting the target state in our statemachine by calling execute()
    template <typename Statemachine>
    void execute(Statemachine& statemachine) {
        statemachine.template set_state<State>();
    }
};

// an invalid state transition which has an emptye execute() function
// we need this (guess what) for all transitions we wont support
struct invalid_state_transition 
{
    template <typename Machine>
    void execute(Machine&) { }
};
