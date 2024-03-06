#pragma once
#include <iostream>       // std::cout
#include <thread>         // std::thread
#include <variant>        // std::variant

#include "WaferAlignment.hpp"

#include "domain/Position.hpp"
#include "domain/Measurement.hpp"
#include "domain/WaferHeightMap.hpp"

namespace WaferAlignmentCommands
{
  struct DummyMethod 
  {
    std::string waferId;
  };

  struct AlignWafer 
  {
    std::string waferId;
  };

  using Command = std::variant<DummyMethod, AlignWafer>;

  class CommandExecutor
  {
    public:
    CommandExecutor(WaferAlignment::WaferAlignment& waferAlignment) : waferAlignment_(waferAlignment){};

    void operator()(const AlignWafer& cmd)
    {
      waferAlignment_.alignWafer(cmd.waferId);
    }

    void operator()(const DummyMethod& cmd)
    {
      waferAlignment_.dummyMethod(cmd.waferId);
    }

  private:
    WaferAlignment::WaferAlignment& waferAlignment_;
  };
}
