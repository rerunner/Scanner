#pragma once

#include <list>
#include "hiberlite.h"
#include "domain/base/AggregateRootBase.hpp"

class Exposure : public AggregateRootBase
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
};

// Boilerplate
HIBERLITE_EXPORT_CLASS(Exposure)
