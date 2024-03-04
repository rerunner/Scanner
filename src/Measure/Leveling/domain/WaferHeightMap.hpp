#pragma once
#include <list>
#include "hiberlite.h"
#include "domain/base/AggregateRootBase.hpp"
#include "Measurement.hpp" //Value Object


class WaferHeightMap : public AggregateRootBase
{
private:
  std::list<Measurement> measurements_;
  
  //Boilerplate start
  friend class hiberlite::access;
  template<class Archive>
  void hibernate(Archive & ar)
  {
    ar & HIBERLITE_NVP(id_); // From Base class
    ar & HIBERLITE_NVP(measurements_);
  }
  //Boilerplate end
public:
  WaferHeightMap() : AggregateRootBase(){};
  WaferHeightMap(Measurement m) : AggregateRootBase()
  {
    measurements_.push_back(m); //First entry
  }

  std::list<Measurement> GetHeightMap(void){return measurements_;}

  void AddMeasurement(Measurement m) { measurements_.push_back(m); }

};

// Boilerplate
HIBERLITE_EXPORT_CLASS(WaferHeightMap)

