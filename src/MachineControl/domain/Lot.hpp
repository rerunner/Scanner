#ifndef LOT_H
#define LOT_H

#include <list>
#include "hiberlite.h"
#include "domain/base/AggregateRootBase.hpp"
#include "FiniteStateMachine.hpp"
#include "Uuid.hpp"

class Lot : public AggregateRootBase
{
private:
  std::list<Uuid> waferIds_;

  //Boilerplate start
  friend class hiberlite::access;
  template<class Archive>
  void hibernate(Archive & ar)
  {
    ar & HIBERLITE_NVP(id_); // From Base class
    ar & HIBERLITE_NVP(waferIds_);
  }
  //Boilerplate end
public:
  Lot() : AggregateRootBase(){}
  void AddWafer(Uuid wId);
  void RemoveWafer(Uuid wId);
};

#endif