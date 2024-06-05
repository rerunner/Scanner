#ifndef WAFERSCANNER_H
#define WAFERSCANNER_H

#include <list>
#include "hiberlite.h"
#include <nlohmann/json.hpp>
#include "domain/base/AggregateRootBase.hpp"

class WaferScanner : public AggregateRootBase
{
private:
  
  //Boilerplate start
  friend class hiberlite::access;
  template<class Archive>
  void hibernate(Archive & ar)
  {
    ar & HIBERLITE_NVP(id_); // From Base class
    ar & HIBERLITE_NVP(parentId_); // From Base class
  }
  //Boilerplate end
public:
  WaferScanner() : AggregateRootBase(){};

  //JSON boilerplate
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(WaferScanner, id_, parentId_)
};

// Boilerplate
HIBERLITE_EXPORT_CLASS(WaferScanner)

#endif