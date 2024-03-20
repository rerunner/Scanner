#include "Position.hpp"

Position::Position(double x, double y) : x_(x), y_(y) 
{
}

bool Position::operator==(const ValueObjectBase& other) const
{
    if (const Position* otherPosition = dynamic_cast<const Position*>(&other))
    {
        return (x_ == otherPosition->x_) && (y_ == otherPosition->y_);
    }
    return false;
}

double Position::GetX() const 
{
    return x_;
}

double Position::GetY() const 
{
    return y_;
}

// Boilerplate
HIBERLITE_EXPORT_CLASS(Position)
