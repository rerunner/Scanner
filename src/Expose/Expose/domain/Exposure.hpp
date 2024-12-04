#pragma once

#if 1
#include "generatedExposeBoundedContextClasses.hpp"
#else
#include <list>
#include "hiberlite.h"
#include <nlohmann/json.hpp>
#include "domain/base/AggregateRootBase.hpp"

class Exposure : public Verdi::AggregateRootBase
{
private:
  float myPrediction;
  
  //Boilerplate start
  friend class hiberlite::access;
  template<class Archive>
  void hibernate(Archive & ar)
  {
    ar & HIBERLITE_NVP(id_); // From Entity Base class
    ar & HIBERLITE_NVP(parentId_); // From Entity Base class
    ar & HIBERLITE_NVP(myPrediction);
  }
  //Boilerplate end
public:
  Exposure(float p = 0.0) : AggregateRootBase(){myPrediction = p;};
  float GetPrediction(){return myPrediction;}

  //JSON boilerplate
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Exposure, id_, parentId_, myPrediction)
};

// Boilerplate
HIBERLITE_EXPORT_CLASS(Exposure)
#endif // test
