#pragma once

#include <list>
#include "hiberlite.h"
#include "domain/base/AggregateRootBase.hpp"
#include "FiniteStateMachine.hpp"

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
    std::cout << "we are running! \n";
    }

    state_transition_to<Prealigned> on_state_transition(const transition_to_Prealigned&) const {
        std::cout << "Leaving Loaded state with transition to Prealigned state\n";
        return {};
    }

    template<typename Transition>
    invalid_state_transition on_state_transition(const Transition&) const {
        std::cout << "State transition: " <<  typeid(Transition).name() << " is not supported in Loaded state! \n";
        return {};
    }
  };
  struct Prealigned
  {
    void on_update() const {
    std::cout << "we are running! \n";
    }

    state_transition_to<Measured> on_state_transition(const transition_to_Measured&) const {
        std::cout << "Leaving Prealigned state with transition to Measured state\n";
        return {};
    }

    template<typename Transition>
    invalid_state_transition on_state_transition(const Transition&) const {
        std::cout << "State transition: " <<  typeid(Transition).name() << " is not supported in Prealigned state! \n";
        return {};
    }
  };
  struct Measured
  {
    void on_update() const {
    std::cout << "we are running! \n";
    }

    state_transition_to<ApprovedForExpose> on_state_transition(const transition_to_ApprovedForExpose&) const {
        std::cout << "Leaving Measured state with transition to ApprovedForExpose state\n";
        return {};
    }

    template<typename Transition>
    invalid_state_transition on_state_transition(const Transition&) const {
        std::cout << "State transition: " <<  typeid(Transition).name() << " is not supported in Measured state! \n";
        return {};
    }
  };
  struct ApprovedForExpose
  {
    void on_update() const {
    std::cout << "we are running! \n";
    }

    state_transition_to<Exposed> on_state_transition(const transition_to_Exposed&) const {
        std::cout << "Leaving ApprovedForExpose state with transition to Exposed state\n";
        return {};
    }

    template<typename Transition>
    invalid_state_transition on_state_transition(const Transition&) const {
        std::cout << "State transition: " <<  typeid(Transition).name() << " is not supported in ApprovedForExpose state! \n";
        return {};
    }
  };
  struct Exposed
  {
    void on_update() const {
    std::cout << "we are running! \n";
    }

    state_transition_to<Unloaded> on_state_transition(const transition_to_Unloaded&) const {
        std::cout << "Leaving Exposed state with transition to Unloaded state\n";
        return {};
    }

    template<typename Transition>
    invalid_state_transition on_state_transition(const Transition&) const {
        std::cout << "State transition: " <<  typeid(Transition).name() << " is not supported in Exposed state! \n";
        return {};
    }
  };
  struct Unloaded
  {
    void on_update() const {
    std::cout << "we are running! \n";
    }

    state_transition_to<Loaded> on_state_transition(const transition_to_Loaded&) const {
        std::cout << "Leaving Exposed state with transition to Unloaded state\n";
        return {};
    }

    template<typename Transition>
    invalid_state_transition on_state_transition(const Transition&) const {
        std::cout << "State transition: " <<  typeid(Transition).name() << " is not supported in Unloaded state! \n";
        return {};
    }
  };
  struct Rejected
  { // Dummy for now
    void on_update() const {
    std::cout << "we are running! \n";
    }

    state_transition_to<Loaded> on_state_transition(const transition_to_Loaded&) const {
        std::cout << "Leaving Exposed state with transition to Unloaded state\n";
        return {};
    }

    template<typename Transition>
    invalid_state_transition on_state_transition(const Transition&) const {
        std::cout << "State transition: " <<  typeid(Transition).name() << " is not supported in Rejected state! \n";
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

  //Boilerplate start
  friend class hiberlite::access;
  template<class Archive>
  void hibernate(Archive & ar)
  {
    ar & HIBERLITE_NVP(id_); // From Base class
  }
  //Boilerplate end
public:
  Wafer() : AggregateRootBase(){};

  void PreAligned()
    {
        waferStateMachine.on_state_transition(waferState::transition_to_Prealigned{});
    }

};

// Boilerplate
HIBERLITE_EXPORT_CLASS(Wafer)
