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
    ar & HIBERLITE_NVP(id_); // From Base class
    ar & HIBERLITE_NVP(myPrediction); // From Base class
  }
  //Boilerplate end
public:
  Exposure(float p = 0.0) : AggregateRootBase(){myPrediction = p;};
  float GetPrediction(){return myPrediction;}
};

// Boilerplate
HIBERLITE_EXPORT_CLASS(Exposure)
