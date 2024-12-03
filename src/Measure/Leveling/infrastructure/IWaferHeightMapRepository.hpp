#pragma once

#include "infrastructure/base/IRepositoryFactory.h"
#include "infrastructure/base/IRepositoryBase.h"

#include "domain/WaferHeightMap.hpp"


class IWaferHeightMapRepository : public Verdi::IRepositoryBase<LevelingContext::WaferHeightMap> {};
