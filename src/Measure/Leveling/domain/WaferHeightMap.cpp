#include "WaferHeightMap.hpp"
#include "GenLogger.hpp"


WaferHeightMap::WaferHeightMap(Uuid wId) : AggregateRootBase()
{
    waferId_ = wId;
    parentId_ = wId;
};

WaferHeightMap::WaferHeightMap(Uuid wId, MarkMeasurement m) : AggregateRootBase()
{
    waferId_ = wId;
    parentId_ = wId;
    measurements_.push_back(m); //First entry
}

std::list<MarkMeasurement> WaferHeightMap::GetHeightMap(void)
{
    return measurements_;
}

void WaferHeightMap::AddMarkMeasurement(MarkMeasurement m) 
{ 
    measurements_.push_back(m); 
}

Uuid WaferHeightMap::GetWaferId()
{
    return waferId_;
}

void WaferHeightMap::LogHeightMap() 
{ 
    for(MarkMeasurement measIter : measurements_)
    {
        GSL::Dprintf(GSL::INFO, "Heightmap Mark Measurement X = ", measIter.GetPosition().GetX(), 
                                ", Y = ", measIter.GetPosition().GetY(), 
                                ", Z = ", measIter.GetZ());
    }
}

// Boilerplate
HIBERLITE_EXPORT_CLASS(WaferHeightMap)
