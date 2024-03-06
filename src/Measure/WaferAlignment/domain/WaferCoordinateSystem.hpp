#pragma once
#include <list>
#include "hiberlite.h"
#include "domain/base/AggregateRootBase.hpp"
#include "AlignmentMark.hpp" // Value Object
#include "WaferDisplacementCoordinates.hpp" // Value Object


class WaferCoordinateSystem : public AggregateRootBase
{
private:
  std::list<AlignmentMark> alignmentMark_;
  WaferDisplacementCoordinates waferDisplacementCoordinates_;
  
  //Boilerplate start
  friend class hiberlite::access;
  template<class Archive>
  void hibernate(Archive & ar)
  {
    ar & HIBERLITE_NVP(id_); // From Base class
    ar & HIBERLITE_NVP(alignmentMark_);
  }
  //Boilerplate end
public:
  WaferCoordinateSystem() : AggregateRootBase(){};

  WaferDisplacementCoordinates GetWaferDisplacementCoordinates(){return waferDisplacementCoordinates_;};
};