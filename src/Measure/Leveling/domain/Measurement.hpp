#ifndef MEASUREMENT_H
#define MEASUREMENT_H

//#include "hiberlite.h"
#include "domain/base/ValueObjectBase.hpp"
#include "Position.hpp"


class Measurement : public ValueObjectBase
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
  Measurement();
  Measurement(Position position, double zvalue = 0.0);
  
  bool operator==(const ValueObjectBase& other) const;

  Position GetPosition() const;
  double GetZ() const;
};

#endif //MEASUREMENT_H