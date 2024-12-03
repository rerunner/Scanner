#include "MarkMeasurement.hpp"

namespace LevelingContext {

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

} // namespace LevelingContext

// Boilerplate
HIBERLITE_EXPORT_CLASS(LevelingContext::MarkMeasurement)
