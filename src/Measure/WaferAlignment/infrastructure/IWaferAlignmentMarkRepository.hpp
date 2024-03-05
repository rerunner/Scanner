#pragma once

#include "infrastructure/base/IRepositoryFactory.h"
#include "infrastructure/base/IRepositoryBase.h"

#include "domain/AlignmentMark.hpp"


class IAlignmentMarkRepository : public IRepositoryBase<AlignmentMark> {};
