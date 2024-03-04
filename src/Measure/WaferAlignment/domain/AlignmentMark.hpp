#pragma once

#include "domain/base/ValueObjectBase.hpp"
#include "MarkPosition.hpp"
#include "MarkMeasurementSpecification.hpp"

class AlignmentMark : public ValueObjectBase
{
private:
  MarkPosition markPosition_;
  MarkMeasurementSpecification markMeasurementSpecification_;
  
  //Boilerplate start
  friend class hiberlite::access;
  template<class Archive>
  void hibernate(Archive & ar)
  {
    ar & HIBERLITE_NVP(markPosition_);
    ar & HIBERLITE_NVP(markMeasurementSpecification_);
  }
  //Boilerplate end
  
public:
  AlignmentMark(MarkMeasurementSpecification markMeasurementSpecification): markMeasurementSpecification_(markMeasurementSpecification) {}

  bool operator==(const ValueObjectBase& other) const override
  {
    return false;
  }

  MarkPosition GetAlignedPosition() const {return markPosition_;}
};

// Boilerplate
HIBERLITE_EXPORT_CLASS(AlignmentMark)

