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

void WaferHeightMap::LogHeightMap() 
{ 
    for(Measurement measIter : measurements_)
    {
        std::cout << "Heightmap Measurement X = " << measIter.GetPosition().GetX();
        std::cout << ", Y = " << measIter.GetPosition().GetY();
        std::cout << ", Z = " << measIter.GetZ() << std::endl;
    }
}

// Boilerplate
HIBERLITE_EXPORT_CLASS(WaferHeightMap)
