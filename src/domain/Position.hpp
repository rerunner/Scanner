#pragma once

#include "base/ValueObjectBase.hpp"


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
  Position(double x = 0.0, double y = 0.0) : x_(x), y_(y) {}

  bool operator==(const ValueObjectBase& other) const override
  {
    if (const Position* otherPosition = dynamic_cast<const Position*>(&other))
    {
      return (x_ == otherPosition->x_) && (y_ == otherPosition->y_);
    }
    return false;
  }

  double GetX() const {return x_;}
  double GetY() const {return y_;}
};

// Boilerplate
HIBERLITE_EXPORT_CLASS(Position)

