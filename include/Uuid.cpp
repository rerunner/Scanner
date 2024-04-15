#include "Uuid.hpp"

Uuid::Uuid ()
{
uuid_ = boost::lexical_cast<std::string>(generateUuid());
}

Uuid::Uuid (std::string withUuid_)
{
uuid_ = withUuid_;
}

const std::string Uuid::Get() const
{
return uuid_;
}

// Boilerplate
HIBERLITE_EXPORT_CLASS(Uuid)
