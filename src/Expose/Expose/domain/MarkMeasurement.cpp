#include "MarkMeasurement.hpp"

namespace ExposeContext {

MarkMeasurement::MarkMeasurement(Position position, double zvalue) : ValueObjectBase()
{ position_ = position;
  z_ = zvalue;
}

bool MarkMeasurement::operator==(const ValueObjectBase& other) const
{ //TODO
  return false;
}

Position MarkMeasurement::GetPosition(void)
{ return position_;
}

double MarkMeasurement::GetZ()
{ return z_;
}

} // namespace ExposeContext

// Boilerplate
HIBERLITE_EXPORT_CLASS(ExposeContext::MarkMeasurement)
