#include "Exposure.hpp"

namespace ExposeContext {

Exposure::Exposure(float p) : AggregateRootBase()
{   myPrediction = p;
};

float Exposure::GetPrediction()
{   return myPrediction;
}

} // namespace ExposeContext

// Boilerplate
HIBERLITE_EXPORT_CLASS(ExposeContext::Exposure)
