#include "Measurement.hpp"

Measurement::Measurement() : ValueObjectBase() 
{
};
  
Measurement::Measurement(Position position, double zvalue) : ValueObjectBase()
{
    position_ = position;
    z_ = zvalue;
}

bool Measurement::operator==(const ValueObjectBase& other) const
  {
    //TODO
    return false;
  }

Position Measurement::GetPosition() const 
{
    return position_;
}

double Measurement::GetZ() const 
{
    return z_;
}

// Boilerplate
HIBERLITE_EXPORT_CLASS(Measurement)
