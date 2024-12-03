#include "WaferHeightMap.hpp"

namespace LevelingContext {

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

} // namespace LevelingContext

// Boilerplate
HIBERLITE_EXPORT_CLASS(LevelingContext::WaferHeightMap)
