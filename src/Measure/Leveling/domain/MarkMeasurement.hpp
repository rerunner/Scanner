#pragma once

//#include "hiberlite.h"
#include "domain/base/ValueObjectBase.hpp"
#include "Position.hpp"


class MarkMeasurement : public ValueObjectBase
{
private:
  double z_;
  Position position_;

  //Boilerplate start
  friend class hiberlite::access;
  template<class Archive>
  void hibernate(Archive & ar)
  {
    ar & HIBERLITE_NVP(z_);
    ar & HIBERLITE_NVP(position_);
  }
  //Boilerplate end
  
public:
  MarkMeasurement();
  MarkMeasurement(Position position, double zvalue = 0.0);
  
  bool operator==(const ValueObjectBase& other) const;

  Position GetPosition() const;
  double GetZ() const;
};
