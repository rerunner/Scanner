#include "WaferHeightMap.hpp"


WaferHeightMap::WaferHeightMap() : AggregateRootBase()
{
};

WaferHeightMap::WaferHeightMap(Measurement m) : AggregateRootBase()
{
    measurements_.push_back(m); //First entry
}

std::list<Measurement> WaferHeightMap::GetHeightMap(void)
{
    return measurements_;
}

void WaferHeightMap::AddMeasurement(Measurement m) 
{ 
    measurements_.push_back(m); 
}

// Boilerplate
HIBERLITE_EXPORT_CLASS(WaferHeightMap)
