#pragma once

#include <list>
#include "hiberlite.h"
#include "domain/base/AggregateRootBase.hpp"
#include "FiniteStateMachine.hpp"

// Life of a Wafer: Loaded -> Prealigned -> Measured -> Approved for expose side -> exposed -> Unloaded
namespace waferState {
  struct Loaded {};
  struct Prealigned {};
  struct Measured {};
  struct ApprovedForExpose{};
  struct Exposed {};
  struct Unloaded {};
  struct Rejected {};
}

  using StateVariant = std::variant<waferState::Loaded, 
                                  waferState::Prealigned, 
                                  waferState::Measured,
                                  waferState::ApprovedForExpose,
                                  waferState::Exposed,
                                  waferState::Unloaded,
                                  waferState::Rejected >;

class Wafer : public AggregateRootBase
{
private:
  
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

};

// Boilerplate
HIBERLITE_EXPORT_CLASS(Wafer)
