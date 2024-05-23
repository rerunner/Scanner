#include "WaferHeightMap.hpp"
#include "GenLogger.hpp"


WaferHeightMap::WaferHeightMap(Uuid wId) : AggregateRootBase()
{
    waferId_ = wId;
    parentId_ = wId;
};

WaferHeightMap::WaferHeightMap(Uuid wId, Measurement m) : AggregateRootBase()
{
    waferId_ = wId;
    parentId_ = wId;
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

Uuid WaferHeightMap::GetWaferId()
{
    return waferId_;
}

void WaferHeightMap::LogHeightMap() 
{ 
    for(Measurement measIter : measurements_)
    {
        GSL::Dprintf(GSL::INFO, "Heightmap Measurement X = ", measIter.GetPosition().GetX(), 
                                ", Y = ", measIter.GetPosition().GetY(), 
                                ", Z = ", measIter.GetZ());
    }
}

// Boilerplate
HIBERLITE_EXPORT_CLASS(WaferHeightMap)
