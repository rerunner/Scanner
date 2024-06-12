#include "Uuid.hpp"

Uuid::Uuid ()
{
uuid_ = boost::lexical_cast<std::string>(generateUuid());
}

Uuid::Uuid (std::string withUuid_)
{
uuid_ = withUuid_;
}

bool Uuid::operator==(const ValueObjectBase& other) const
{
    if (const Uuid* otherUuid = dynamic_cast<const Uuid*>(&other))
    {
        return (uuid_ == otherUuid->Get());
    }
    return false;
}

const std::string Uuid::Get() const
{
return uuid_;
}

// Boilerplate
HIBERLITE_EXPORT_CLASS(Uuid)
