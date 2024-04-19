#ifndef MACHINECONTROL_H
#define MACHINECONTROL_H

#include <iostream>
#include <source_location>
#include "domain/Lot.hpp"
#include "domain/Wafer.hpp"
#include "FiniteStateMachine.hpp"

#include <cppkafka/cppkafka.h>

#include "GenLogger.hpp"

namespace MachineControl
{
    // specific state transition types we support
    struct transition_to_Executing{};
    struct transition_to_Idle{};

    // Statemachine start
    // SEMI Machine states
    // state definitions
    namespace state {
        struct Error;
        struct Idle;
        struct Executing;
    
        struct Error 
        { 
            void on_update() const {
            GSL::Dprintf(GSL::INFO, "MC is running!");
            }

            state_transition_to<Idle> on_state_transition(const transition_to_Idle&) const {
                GSL::Dprintf(GSL::INFO, "Leaving Error state with transition to Idle state");
                return {};
            }

            template<typename Transition>
            invalid_state_transition on_state_transition(const Transition&) const {
                GSL::Dprintf(GSL::ERROR, "State transition: ", typeid(Transition).name(), " is not supported in Error state!");
                return {};
            }
        };
        struct Idle 
        { 
            // regular on update call 
            void on_update() const {
                GSL::Dprintf(GSL::INFO, "still waiting");
            }

            // specific transition to run, where we return the concrete state transition to Executing
            // to distinguish different state transitions, we use an empty function argument here
            state_transition_to<Executing> on_state_transition(const transition_to_Executing&) const{
                GSL::Dprintf(GSL::INFO, "Leaving Idle state with transition to Executing state");
                return {};
            }

            // a template function to indicate all non supported state transitions.
            template<typename Transition>
            invalid_state_transition on_state_transition(const Transition&) const {
                GSL::Dprintf(GSL::ERROR, "State transition: ", typeid(Transition).name(), " is not supported in Idle state!");
                return {};
            }
        };
        struct Executing 
        { 
            void on_update() const {
            GSL::Dprintf(GSL::INFO, "Machine Control is running!");
            }

            state_transition_to<Idle> on_state_transition(const transition_to_Idle&) const {
                GSL::Dprintf(GSL::INFO, "Leaving Executing state with transition to Idle state");
                return {};
            }

            template<typename Transition>
            invalid_state_transition on_state_transition(const Transition&) const {
                GSL::Dprintf(GSL::ERROR, "State transition: ", typeid(Transition).name(), " is not supported in Executing state!");
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
        // Kafka part
        std::unique_ptr<cppkafka::Configuration> kafkaConfig;
        std::unique_ptr<cppkafka::Consumer> kafkaConsumer;
        std::thread eventListenerThread;
        std::list<Wafer> lotWafers;
        void eventListenerThreadHandler();
        bool quit_;
    public:
        MachineControl();
        ~MachineControl();
        void Initialize();
        void Execute();
    };
}

#endif