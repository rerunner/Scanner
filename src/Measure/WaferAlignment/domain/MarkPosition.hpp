#pragma once

#include "domain/base/ValueObjectBase.hpp"


class MarkPosition : public ValueObjectBase
{
private:
  double xpos_;
  double ypos_;

  //Boilerplate start
  friend class hiberlite::access;
  template<class Archive>
  void hibernate(Archive & ar)
  {
    ar & HIBERLITE_NVP(xpos_);
    ar & HIBERLITE_NVP(ypos_);
  }
  //Boilerplate end
  
public:
  MarkPosition(double xpos = 0.0, double ypos = 0.0) : xpos_(xpos), ypos_(ypos) {}

  bool operator==(const ValueObjectBase& other) const override
  {
    if (const Position* otherPosition = dynamic_cast<const Position*>(&other))
    {
      return (xpos_ == otherPosition->xpos_) && (ypos_ == otherPosition->ypos_);
    }
    return false;
  }

  double GetX() const {return xpos_;}
  double GetY() const {return ypos_;}
};

// Boilerplate
HIBERLITE_EXPORT_CLASS(MarkPosition)

