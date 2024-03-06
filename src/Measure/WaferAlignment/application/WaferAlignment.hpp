#pragma once
#include <iostream>       // std::cout
#include <thread>         // std::thread

#include "infrastructure/base/RepositoryFactory.h"

#include "domain/Position.hpp"
#include "domain/Measurement.hpp"
#include "domain/WaferHeightMap.hpp"
#include "infrastructure/IWaferHeightMapRepository.hpp"

namespace WaferAlignment
{
  class WaferAlignment
  {
    public:
    WaferAlignment(){}

    void dummyMethod(std::string waferId){}

    void alignWafer(std::string waferId){}

  };
}