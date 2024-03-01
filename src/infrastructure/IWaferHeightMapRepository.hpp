#pragma once

#include "base/IRepositoryFactory.h"
#include "base/IRepositoryBase.h"

#include "domain/WaferHeightMap.hpp"


class IWaferHeightMapRepository : public IRepositoryBase<WaferHeightMap> {};
