#ifndef LOT_H
#define LOT_H

#include <list>
#include <cppkafka/cppkafka.h>
#include "hiberlite.h"
#include "domain/base/AggregateRootBase.hpp"
#include "FiniteStateMachine.hpp"
#include "FiniteStateMachine.hpp"
#include "Uuid.hpp"
#include "GenLogger.hpp"

namespace lotState {
  struct Loaded;
  struct Started;
  struct Finished;
  struct Rejected;
  struct Unloaded;

  // specific state transition types we support
  struct transition_to_Loaded{};
  struct transition_to_Started{};
  struct transition_to_Finished{};
  struct transition_to_Rejected{};
  struct transition_to_Unloaded{};
  
  struct Loaded 
  {
    void on_update() const {
    GSL::Dprintf(GSL::DEBUG, "we are running!");
    }

    state_transition_to<Started> on_state_transition(const transition_to_Started&) const {
        GSL::Dprintf(GSL::DEBUG, "Leaving Loaded state with transition to Started state");
        return {};
    }

    template<typename Transition>
    invalid_state_transition on_state_transition(const Transition&) const {
        GSL::Dprintf(GSL::ERROR, "State transition: ", typeid(Transition).name(), " is not supported in Loaded state!");
        return {};
    }
  };
  struct Started
  {
    void on_update() const {
    GSL::Dprintf(GSL::DEBUG, "we are running!");
    }

    state_transition_to<Finished> on_state_transition(const transition_to_Finished&) const {
        GSL::Dprintf(GSL::DEBUG, "Leaving Prealigned state with transition to Finished state");
        return {};
    }

    template<typename Transition>
    invalid_state_transition on_state_transition(const Transition&) const {
        GSL::Dprintf(GSL::ERROR, "State transition: ", typeid(Transition).name(), " is not supported in Started state!");
        return {};
    }
  };
  struct Finished
  {
    void on_update() const {
    GSL::Dprintf(GSL::DEBUG, "we are running!");
    }

    state_transition_to<Unloaded> on_state_transition(const transition_to_Unloaded&) const {
        GSL::Dprintf(GSL::DEBUG, "Leaving Measured state with transition to Unloaded state");
        return {};
    }

    template<typename Transition>
    invalid_state_transition on_state_transition(const Transition&) const {
        GSL::Dprintf(GSL::ERROR, "State transition: ", typeid(Transition).name(), " is not supported in Finished state!");
        return {};
    }
  };
  struct Rejected
  { // Dummy for now
    void on_update() const {
    GSL::Dprintf(GSL::DEBUG, "we are running!");
    }

    state_transition_to<Unloaded> on_state_transition(const transition_to_Unloaded&) const {
        GSL::Dprintf(GSL::DEBUG, "Leaving Rejected state with transition to Unloaded state");
        return {};
    }

    template<typename Transition>
    invalid_state_transition on_state_transition(const Transition&) const {
        GSL::Dprintf(GSL::ERROR, "State transition: ", typeid(Transition).name(), " is not supported in Rejected state!");
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

    template<typename Transition>
    invalid_state_transition on_state_transition(const Transition&) const {
        GSL::Dprintf(GSL::ERROR, "State transition: ", typeid(Transition).name(), " is not supported in Unloaded state!");
        return {};
    }
  };
}

using lot_state_machine = state_machine<  lotState::Loaded, 
                                          lotState::Started, 
                                          lotState::Finished,
                                          lotState::Unloaded,
                                          lotState::Rejected >;

class Lot : public AggregateRootBase
{
private:
  lot_state_machine lotStateMachine;
  std::string state;
  std::list<Uuid> waferIds_;

  //Boilerplate start
  friend class hiberlite::access;
  template<class Archive>
  void hibernate(Archive & ar)
  {
    ar & HIBERLITE_NVP(id_); // From Base class
    ar & HIBERLITE_NVP(parentId_); // From Base class
    ar & HIBERLITE_NVP(waferIds_);
  }
  //Boilerplate end

  // Kafka part
  cppkafka::Configuration *kafkaConfig;
  cppkafka::Producer *kafkaProducer;
  void stateChangePublisher();
public:
  Lot();
  virtual ~Lot(){}
  void AddWafer(Uuid wId);
  void RemoveWafer(Uuid wId);

  std::string GetCurrentState() const {return state;}

  void Started()
  {
    lotStateMachine.on_state_transition(lotState::transition_to_Started{});
    state = "Started";
    stateChangePublisher();
  }
  void Finished()
  {
    lotStateMachine.on_state_transition(lotState::transition_to_Finished{});
    state = "Finished";
    stateChangePublisher();
  }
  void Unloaded()
  {
    lotStateMachine.on_state_transition(lotState::transition_to_Unloaded{});
    state = "Unloaded";
    stateChangePublisher();
  }
};

#endif