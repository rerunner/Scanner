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
  std::string waferId_;
  
  //Boilerplate start
  friend class hiberlite::access;
  template<class Archive>
  void hibernate(Archive & ar)
  {
    ar & HIBERLITE_NVP(id_); // From Base class
    ar & HIBERLITE_NVP(measurements_);
    ar & HIBERLITE_NVP(waferId_);
  }
  //Boilerplate end
public:
  WaferHeightMap() : AggregateRootBase(){waferId_ = "0";}
  WaferHeightMap(std::string wId);
  WaferHeightMap(std::string wId, Measurement m);

  std::list<Measurement> GetHeightMap(void);
  void AddMeasurement(Measurement m);
  std::string GetWaferId();
  void LogHeightMap();
};

#endif /* WAFERHEIGHTMAP_H */