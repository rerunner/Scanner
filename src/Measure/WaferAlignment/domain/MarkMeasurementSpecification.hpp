#pragma once

#include "domain/base/ValueObjectBase.hpp"
#include "MarkPosition.hpp"


class MarkMeasurementSpecification : public ValueObjectBase
{
private:
  MarkPosition expectedPosition_;
  int alignmentStep_;

  //Boilerplate start
  friend class hiberlite::access;
  template<class Archive>
  void hibernate(Archive & ar)
  {
    ar & HIBERLITE_NVP(expectedPosition_);
    ar & HIBERLITE_NVP(alignmentStep_);
  }
  //Boilerplate end
  
public:
  MarkMeasurementSpecification(int alignmentStep, MarkPosition expectedPosition) : alignmentStep_(alignmentStep), 
                                                                                   expectedPosition_(expectedPosition) {}

  bool operator==(const ValueObjectBase& other) const override
  {
    return false;
  }

};

// Boilerplate
HIBERLITE_EXPORT_CLASS(MarkMeasurementSpecification)

