#ifndef WAFER_H
#define WAFER_H

#include <list>
#include <cppkafka/cppkafka.h>
#include "hiberlite.h"
#include "domain/base/AggregateRootBase.hpp"
#include "FiniteStateMachine.hpp"
#include "Uuid.hpp"
#include "GenLogger.hpp"

// Life of a Wafer: Loaded -> Prealigned -> Measured -> Approved for expose side -> exposed -> Unloaded
namespace waferState {
  struct Loaded;
  struct Prealigned;
  struct Measured;
  struct ApprovedForExpose;
  struct Exposed;
  struct Unloaded;
  struct Rejected;

  // specific state transition types we support
  struct transition_to_Loaded{};
  struct transition_to_Prealigned{};
  struct transition_to_Measured{};
  struct transition_to_ApprovedForExpose{};
  struct transition_to_Exposed{};
  struct transition_to_Unloaded{};
  struct transition_to_Rejected{};
  
  struct Loaded 
  {
    void on_update() const {
    GSL::Dprintf(GSL::INFO, "we are running!");
    }

    state_transition_to<Prealigned> on_state_transition(const transition_to_Prealigned&) const {
        GSL::Dprintf(GSL::INFO, "Leaving Loaded state with transition to Prealigned state");
        return {};
    }

    template<typename Transition>
    invalid_state_transition on_state_transition(const Transition&) const {
        GSL::Dprintf(GSL::ERROR, "State transition: ", typeid(Transition).name(), " is not supported in Loaded state!");
        return {};
    }
  };
  struct Prealigned
  {
    void on_update() const {
    GSL::Dprintf(GSL::INFO, "we are running!");
    }

    state_transition_to<Measured> on_state_transition(const transition_to_Measured&) const {
        GSL::Dprintf(GSL::INFO, "Leaving Prealigned state with transition to Measured state");
        return {};
    }

    template<typename Transition>
    invalid_state_transition on_state_transition(const Transition&) const {
        GSL::Dprintf(GSL::ERROR, "State transition: ", typeid(Transition).name(), " is not supported in Prealigned state!");
        return {};
    }
  };
  struct Measured
  {
    void on_update() const {
    GSL::Dprintf(GSL::INFO, "we are running!");
    }

    state_transition_to<ApprovedForExpose> on_state_transition(const transition_to_ApprovedForExpose&) const {
        GSL::Dprintf(GSL::INFO, "Leaving Measured state with transition to ApprovedForExpose state");
        return {};
    }

    template<typename Transition>
    invalid_state_transition on_state_transition(const Transition&) const {
        GSL::Dprintf(GSL::ERROR, "State transition: ", typeid(Transition).name(), " is not supported in Measured state!");
        return {};
    }
  };
  struct ApprovedForExpose
  {
    void on_update() const {
    GSL::Dprintf(GSL::INFO, "we are running!");
    }

    state_transition_to<Exposed> on_state_transition(const transition_to_Exposed&) const {
        GSL::Dprintf(GSL::INFO, "Leaving ApprovedForExpose state with transition to Exposed state");
        return {};
    }

    template<typename Transition>
    invalid_state_transition on_state_transition(const Transition&) const {
        GSL::Dprintf(GSL::ERROR, "State transition: ", typeid(Transition).name(), " is not supported in ApprovedForExpose state!");
        return {};
    }
  };
  struct Exposed
  {
    void on_update() const {
    GSL::Dprintf(GSL::INFO, "we are running!");
    }

    state_transition_to<Unloaded> on_state_transition(const transition_to_Unloaded&) const {
        GSL::Dprintf(GSL::INFO, "Leaving Exposed state with transition to Unloaded state");
        return {};
    }

    template<typename Transition>
    invalid_state_transition on_state_transition(const Transition&) const {
        GSL::Dprintf(GSL::ERROR, "State transition: ", typeid(Transition).name(), " is not supported in Exposed state!");
        return {};
    }
  };
  struct Unloaded
  {
    void on_update() const {
    GSL::Dprintf(GSL::INFO, "we are running!");
    }

    state_transition_to<Loaded> on_state_transition(const transition_to_Loaded&) const {
        GSL::Dprintf(GSL::INFO, "Leaving Unloaded state with transition to Loaded state");
        return {};
    }

    template<typename Transition>
    invalid_state_transition on_state_transition(const Transition&) const {
        GSL::Dprintf(GSL::ERROR, "State transition: ", typeid(Transition).name(), " is not supported in Unloaded state!");
        return {};
    }
  };
  struct Rejected
  { // Dummy for now
    void on_update() const {
    GSL::Dprintf(GSL::INFO, "we are running!");
    }

    state_transition_to<Unloaded> on_state_transition(const transition_to_Unloaded&) const {
        GSL::Dprintf(GSL::INFO, "Leaving Rejected state with transition to Unloaded state");
        return {};
    }

    template<typename Transition>
    invalid_state_transition on_state_transition(const Transition&) const {
        GSL::Dprintf(GSL::ERROR, "State transition: ", typeid(Transition).name(), " is not supported in Rejected state!");
        return {};
    }
  };
}

using wafer_state_machine = state_machine<  waferState::Loaded, 
                                            waferState::Prealigned, 
                                            waferState::Measured,
                                            waferState::ApprovedForExpose,
                                            waferState::Exposed,
                                            waferState::Unloaded,
                                            waferState::Rejected >;

class Wafer : public AggregateRootBase
{
private:
  wafer_state_machine waferStateMachine;
  std::unique_ptr<Uuid> parentLot_;

  //Boilerplate start
  friend class hiberlite::access;
  template<class Archive>
  void hibernate(Archive & ar)
  {
    ar & HIBERLITE_NVP(id_); // From Base class
    ar & HIBERLITE_NVP(waferStateMachine);
  }
  //Boilerplate end

  // Kafka part
  std::unique_ptr<cppkafka::Configuration> kafkaConfig;
  std::unique_ptr<cppkafka::Producer> kafkaProducer;
  void stateChangePublisher(std::string state);
public:
  Wafer() : AggregateRootBase(){parentLot_ = nullptr;};
  Wafer(Uuid lotId) : AggregateRootBase(){parentLot_ = std::make_unique<Uuid>(lotId);};
  virtual ~Wafer(){}

  Uuid GetLotId() const {return *parentLot_;}

  void PreAligned()
  {
    waferStateMachine.on_state_transition(waferState::transition_to_Prealigned{});
    stateChangePublisher("PreAligned");
  }
  void Measured()
  {
    waferStateMachine.on_state_transition(waferState::transition_to_Measured{});
    stateChangePublisher("Measured");
  }
  void ApprovedForExpose()
  {
    waferStateMachine.on_state_transition(waferState::transition_to_ApprovedForExpose{});
    stateChangePublisher("ApprovedForExpose");
  }
  void Exposed()
  {
    waferStateMachine.on_state_transition(waferState::transition_to_Exposed{});
    stateChangePublisher("Exposed");
  }
  void Unloaded()
  {
    waferStateMachine.on_state_transition(waferState::transition_to_Unloaded{});
    stateChangePublisher("Unloaded");
  }
};

#endif