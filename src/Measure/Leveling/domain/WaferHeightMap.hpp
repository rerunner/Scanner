#pragma once

#include <list>
//#include "hiberlite.h"
#include <nlohmann/json.hpp>
#include "domain/base/AggregateRootBase.hpp"
#include "MarkMeasurement.hpp"


class WaferHeightMap : public AggregateRootBase
{
private:
  std::list<MarkMeasurement> measurements_;
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
  WaferHeightMap(Uuid wId, MarkMeasurement m);
  virtual ~WaferHeightMap(){};

  std::list<MarkMeasurement> GetHeightMap(void);
  void AddMarkMeasurement(MarkMeasurement m);
  Uuid GetWaferId();
  void LogHeightMap();

  //JSON boilerplate
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(WaferHeightMap, id_, parentId_, measurements_, waferId_)
};
