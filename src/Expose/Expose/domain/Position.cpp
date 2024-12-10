#include "Position.hpp"

namespace ExposeContext {

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

} // namespace ExposeContext

// Boilerplate
HIBERLITE_EXPORT_CLASS(ExposeContext::Position)
