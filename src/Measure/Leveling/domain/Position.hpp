#ifndef POSITION_H
#define POSITION_H

#include "hiberlite.h"
#include <nlohmann/json.hpp>
#include "domain/base/ValueObjectBase.hpp"

class Position : public ValueObjectBase
{
private:
  double x_;
  double y_;

  //Boilerplate start
  friend class hiberlite::access;
  template<class Archive>
  void hibernate(Archive & ar)
  {
    ar & HIBERLITE_NVP(x_);
    ar & HIBERLITE_NVP(y_);
  }
  //Boilerplate end
  
public:
  Position(double x = 0.0, double y = 0.0);

  bool operator==(const ValueObjectBase& other) const override;

  double GetX() const;
  double GetY() const;

  //JSON boilerplate
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Position, x_, y_)
};

#endif // POSITION_H