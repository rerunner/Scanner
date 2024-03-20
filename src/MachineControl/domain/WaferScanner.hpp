#ifndef WAFERSCANNER_H
#define WAFERSCANNER_H

#include <list>
#include "hiberlite.h"
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
  }
  //Boilerplate end
public:
  WaferScanner() : AggregateRootBase(){};

};

// Boilerplate
HIBERLITE_EXPORT_CLASS(WaferScanner)

#endif