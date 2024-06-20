#pragma once

#include "FiniteStateMachine.hpp"
#include "GenLogger.hpp"

enum StationEnumType
{
    MeasureStation,
    ExposeStation
};

namespace stationState {
  struct Idle;
  struct Processing;

  // specific state transition types we support
  struct transition_to_Idle{};
  struct transition_to_Processing{};
  
  struct Idle
  {
    void on_update() const {
    GSL::Dprintf(GSL::DEBUG, "we are running!");
    }

    state_transition_to<Processing> on_state_transition(const transition_to_Processing&) const {
        GSL::Dprintf(GSL::DEBUG, "Leaving Idle state with transition to Processing state");
        return {};
    }

    template<typename Transition>
    invalid_state_transition on_state_transition(const Transition&) const {
        GSL::Dprintf(GSL::ERROR, "State transition: ", typeid(Transition).name(), " is not supported in Idle state!");
        return {};
    }
  };

  struct Processing
  {
    void on_update() const {
    GSL::Dprintf(GSL::DEBUG, "we are running!");
    }

    state_transition_to<Idle> on_state_transition(const transition_to_Idle&) const {
        GSL::Dprintf(GSL::DEBUG, "Leaving Processsing state with transition to Idle state");
        return {};
    }

    template<typename Transition>
    invalid_state_transition on_state_transition(const Transition&) const {
        GSL::Dprintf(GSL::ERROR, "State transition: ", typeid(Transition).name(), " is not supported in Processing state!");
        return {};
    }
  }; 
}

using station_state_machine = state_machine<  stationState::Idle, 
                                              stationState::Processing >;

class Station
{
    private:
    station_state_machine stationStateMachine;
    std::string state;
    bool commandCompleted;
    public:
    Station(){state = "Idle";commandCompleted = true;}
    void ProcessWafer()
    {
        stationStateMachine.on_state_transition(stationState::transition_to_Processing{});
        state = "Processing";
    }
    void ReturnToIdle()
    {
        stationStateMachine.on_state_transition(stationState::transition_to_Idle{});
        state = "Idle";
    }
    std::string const GetStationState(){return state;}
    void DoCommand(){if (commandCompleted) {commandCompleted = false;} else {GSL::Dprintf(GSL::FATAL, "Previous Command not yet completed!");}}
    void CommandHasCompleted(){commandCompleted = true;} // To Be Removed
    bool const GetCommandCompletedState(){return commandCompleted;}
};