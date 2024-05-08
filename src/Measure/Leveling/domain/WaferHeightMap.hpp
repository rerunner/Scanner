#ifndef WAFERHEIGHTMAP_H
#define WAFERHEIGHTMAP_H

#include <list>
//#include "hiberlite.h"
#include "domain/base/AggregateRootBase.hpp"
#include "Measurement.hpp" //Value Object


class WaferHeightMap : public AggregateRootBase
{
private:
  std::list<Measurement> measurements_;
  Uuid waferId_;
  
  //Boilerplate start
  friend class hiberlite::access;
  template<class Archive>
  void hibernate(Archive & ar)
  {
    ar & HIBERLITE_NVP(id_); // From Entity Base class
    ar & HIBERLITE_NVP(parentId_); // From Entity Base class
    ar & HIBERLITE_NVP(measurements_);
    ar & HIBERLITE_NVP(waferId_);
  }
  //Boilerplate end
public:
  WaferHeightMap() : AggregateRootBase(){}
  WaferHeightMap(Uuid wId);
  WaferHeightMap(Uuid wId, Measurement m);
  virtual ~WaferHeightMap(){};

  std::list<Measurement> GetHeightMap(void);
  void AddMeasurement(Measurement m);
  Uuid GetWaferId();
  void LogHeightMap();
};

#endif /* WAFERHEIGHTMAP_H */