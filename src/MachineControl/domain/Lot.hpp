#ifndef LOT_H
#define LOT_H

#include <list>
#include "hiberlite.h"
#include "domain/base/AggregateRootBase.hpp"
#include "FiniteStateMachine.hpp"

class Lot : public AggregateRootBase
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
  Lot() : AggregateRootBase(){};

};

// Boilerplate
HIBERLITE_EXPORT_CLASS(Lot)

#endif