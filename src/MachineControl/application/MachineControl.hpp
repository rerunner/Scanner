#pragma once

#include "FiniteStateMachine.hpp"

// specific state transition types we support
//Initialized, StartExecution, EndExecution, ErrorReceived
struct transition_to_Executing{};
struct transition_to_Idle{};

namespace MachineControl
{
    // Statemachine start
    // SEMI Machine states
    // state definitions
    struct Error;
    struct Idle;
    struct Executing;
    
    namespace state {
        struct Error;
        struct Idle;
        struct Executing;
    }
    
    namespace state {
        struct Error 
        { 
            void on_update() const {
            std::cout << "we are running! \n";
            }

            state_transition_to<Idle> on_state_transition(const transition_to_Idle&) const {
                std::cout << "leaving Error with transition to Idle \n";
                return {};
            }

            template<typename Transition>
            invalid_state_transition on_state_transition(const Transition&) const {
                std::cout << "state transition: " <<  typeid(Transition).name() << " is not supported in Error! \n";
                return {};
            }
        };
        struct Idle 
        { 
            // regular on update call 
            void on_update() const {
                std::cout << "still waiting \n";
            }

            // specific transition to run, where we return the concrete state transition to Executing
            // to distinguish different state transitions, we use an empty function argument here
            state_transition_to<Executing> on_state_transition(const transition_to_Executing&) const{
                std::cout << "leaving Idle with transition to Executing \n";
                return {};
            }

            // a template function to indicate all non supported state transitions.
            template<typename Transition>
            invalid_state_transition on_state_transition(const Transition&) const {
                std::cout << "state transition: " <<  typeid(Transition).name() << " is not supported in Idle! \n";
                return {};
            }
        };
        struct Executing 
        { 
            void on_update() const {
            std::cout << "we are running! \n";
            }

            state_transition_to<Idle> on_state_transition(const transition_to_Idle&) const {
                std::cout << "leaving run with transition to Idle \n";
                return {};
            }

            template<typename Transition>
            invalid_state_transition on_state_transition(const Transition&) const {
                std::cout << "state transition: " <<  typeid(Transition).name() << " is not supported in run! \n";
                return {};
            }
        };
    }

    // the alias for our state machine with state idle and run
    // the statemachin is initialized with idle
    using machinecontrol_state_machine = state_machine<state::Error, state::Idle, state::Executing>;
    // Statemachine end
    
    ////////////////////
    // Class Declaration
    ////////////////////
    class MachineControl
    {
    private:
        machinecontrol_state_machine machineControlStateMachine;

    public:
        void Initialize();
        void Execute();
    };
}