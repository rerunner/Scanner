#pragma once

#include <list>
#include <cppkafka/cppkafka.h>
#include "hiberlite.h"
#include <nlohmann/json.hpp>
#include "domain/base/AggregateRootBase.hpp"
#include "domain/Station.hpp"
#include "FiniteStateMachine.hpp"
#include "GenLogger.hpp"

namespace chuckState {
  struct Unloaded;
  struct Loaded;
  struct ReadyForSwap;
  struct ReadyForUnloading;

  // specific state transition types we support
  struct transition_to_Unloaded{};
  struct transition_to_Loaded{};
  struct transition_to_ReadyForSwap{};
  struct transition_to_ReadyForUnloading{};

  struct Loaded 
  {
    void on_update() const {
    GSL::Dprintf(GSL::DEBUG, "we are running!");
    }

    state_transition_to<Unloaded> on_state_transition(const transition_to_Unloaded&) const {
        GSL::Dprintf(GSL::DEBUG, "Leaving Loaded state with transition to Unloaded state");
        return {};
    }

    state_transition_to<ReadyForSwap> on_state_transition(const transition_to_ReadyForSwap&) const {
        GSL::Dprintf(GSL::DEBUG, "Leaving Loaded state with transition to ReadyForSwap state");
        return {};
    }

    template<typename Transition>
    invalid_state_transition on_state_transition(const Transition&) const {
        GSL::Dprintf(GSL::ERROR, "State transition: ", typeid(Transition).name(), " is not supported in Loaded state!");
        return {};
    }
  };

  struct ReadyForSwap
  {
    void on_update() const {
    GSL::Dprintf(GSL::DEBUG, "we are running!");
    }

    state_transition_to<Loaded> on_state_transition(const transition_to_Loaded&) const {
        GSL::Dprintf(GSL::DEBUG, "Leaving ReadyForSwap state with transition to Loaded state");
        return {};
    }

    state_transition_to<ReadyForUnloading> on_state_transition(const transition_to_ReadyForUnloading&) const {
        GSL::Dprintf(GSL::DEBUG, "Leaving ReadyForSwap state with transition to ReadyForUnloading state");
        return {};
    }

    template<typename Transition>
    invalid_state_transition on_state_transition(const Transition&) const {
        GSL::Dprintf(GSL::ERROR, "State transition: ", typeid(Transition).name(), " is not supported in ReadyForSwap state!");
        return {};
    }
  };

  struct ReadyForUnloading
  {
    void on_update() const {
    GSL::Dprintf(GSL::DEBUG, "we are running!");
    }

    state_transition_to<Unloaded> on_state_transition(const transition_to_Unloaded&) const {
        GSL::Dprintf(GSL::DEBUG, "Leaving ReadyForUnloading state with transition to Unloaded state");
        return {};
    }

    template<typename Transition>
    invalid_state_transition on_state_transition(const Transition&) const {
        GSL::Dprintf(GSL::ERROR, "State transition: ", typeid(Transition).name(), " is not supported in ReadyForUnloading state!");
        return {};
    }
  };
  
  struct Unloaded
  {
    void on_update() const {
    GSL::Dprintf(GSL::DEBUG, "we are running!");
    }

    state_transition_to<Loaded> on_state_transition(const transition_to_Loaded&) const {
        GSL::Dprintf(GSL::DEBUG, "Leaving Unloaded state with transition to Loaded state");
        return {};
    }

    state_transition_to<ReadyForSwap> on_state_transition(const transition_to_ReadyForSwap&) const {
        GSL::Dprintf(GSL::DEBUG, "Leaving Unloaded state with transition to ReadyForSwap state"); // Only for empty Expose station
        return {};
    }

    template<typename Transition>
    invalid_state_transition on_state_transition(const Transition&) const {
        GSL::Dprintf(GSL::ERROR, "State transition: ", typeid(Transition).name(), " is not supported in Unloaded state!");
        return {};
    }
  };
  
}

using chuck_state_machine = state_machine<  chuckState::Unloaded,
                                            chuckState::Loaded, 
                                            chuckState::ReadyForSwap,
                                            chuckState::ReadyForUnloading >;

class Chuck : public AggregateRootBase
{
private:
  chuck_state_machine chuckStateMachine;
  std::shared_ptr<Uuid> holdingWaferId_;
  std::string state;
  StationEnumType atStation_;

  //Boilerplate start
  friend class hiberlite::access;
  template<class Archive>
  void hibernate(Archive & ar)
  {
    ar & HIBERLITE_NVP(id_); // From Base class
    ar & HIBERLITE_NVP(parentId_); // From Base class
    ar & HIBERLITE_NVP(chuckStateMachine);
  }
  //Boilerplate end

  // Kafka part
  std::unique_ptr<cppkafka::Configuration> kafkaConfig;
  std::unique_ptr<cppkafka::Producer> kafkaProducer;
  //void stateChangePublisher();
public:
  Chuck() : AggregateRootBase(){holdingWaferId_ = nullptr; state = "Unloaded";};
  Chuck(Uuid waferId) : AggregateRootBase(){holdingWaferId_ = std::make_shared<Uuid>(waferId);atStation_ = StationEnumType::MeasureStation; LoadWafer(waferId);};
  virtual ~Chuck(){}

  std::shared_ptr<Uuid> GetWaferId() const {return holdingWaferId_;}
  std::string GetCurrentState() const {return state;}
  const StationEnumType GetStation() const {return atStation_;}
  void SetStation(StationEnumType newStation){atStation_ = newStation;}
  void SwapStation();
  void LoadWafer(Uuid wId);
  void UnloadWafer();
  void SetReadyForSwap();

  //JSON boilerplate
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Chuck, id_, parentId_)
};