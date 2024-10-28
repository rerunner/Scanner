#include "MarkMeasurement.hpp"

using namespace Verdi;

MarkMeasurement::MarkMeasurement() : ValueObjectBase() 
{
};
  
MarkMeasurement::MarkMeasurement(Position position, double zvalue) : ValueObjectBase()
{
    position_ = position;
    z_ = zvalue;
}

bool MarkMeasurement::operator==(const ValueObjectBase& other) const
  {
    //TODO
    return false;
  }

Position MarkMeasurement::GetPosition() const 
{
    return position_;
}

double MarkMeasurement::GetZ() const 
{
    return z_;
}

// Boilerplate
HIBERLITE_EXPORT_CLASS(MarkMeasurement)
