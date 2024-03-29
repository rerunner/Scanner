#include "WaferHeightMap.hpp"


WaferHeightMap::WaferHeightMap(std::string wId) : AggregateRootBase()
{
    waferId_ = wId;
};

WaferHeightMap::WaferHeightMap(std::string wId, Measurement m) : AggregateRootBase()
{
    waferId_ = wId;
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

std::string WaferHeightMap::GetWaferId()
{
    return waferId_;
}

void WaferHeightMap::LogHeightMap() 
{ 
#if 0
    for(Measurement measIter : measurements_)
    {
        std::cout << "Heightmap Measurement X = " << measIter.GetPosition().GetX();
        std::cout << ", Y = " << measIter.GetPosition().GetY();
        std::cout << ", Z = " << measIter.GetZ() << std::endl;
    }
#endif
}

// Boilerplate
HIBERLITE_EXPORT_CLASS(WaferHeightMap)
