#include "WaferHeightMap.hpp"

namespace ExposeContext {

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
    return parentId_;
}

} // namespace ExposeContext

// Boilerplate
HIBERLITE_EXPORT_CLASS(ExposeContext::WaferHeightMap)
