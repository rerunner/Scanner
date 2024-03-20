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
  WaferHeightMap();
  WaferHeightMap(Measurement m);

  std::list<Measurement> GetHeightMap(void);
  void AddMeasurement(Measurement m);
};

#endif /* WAFERHEIGHTMAP_H */