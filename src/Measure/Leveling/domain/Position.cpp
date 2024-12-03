#include "Position.hpp"

namespace LevelingContext {

Position::Position(double x, double y) : xpos_(x), ypos_(y) 
{
}


bool Position::operator==(const ValueObjectBase& other) const
{   if (const Position* otherPosition = dynamic_cast<const Position*>(&other))
    {
        return (xpos_ == otherPosition->xpos_) && (ypos_ == otherPosition->ypos_);
    }
    return false;
}

double Position::GetX()
{   return xpos_;
}

double Position::GetY()
{   return ypos_;
}

} // namespace LevelingContext

// Boilerplate
HIBERLITE_EXPORT_CLASS(LevelingContext::Position)
