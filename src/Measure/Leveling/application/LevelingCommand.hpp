#pragma once
#include <iostream>       // std::cout
#include <thread>         // std::thread
#include <variant>        // std::variant

#include "Leveling.hpp"

#include "domain/Position.hpp"
#include "domain/Measurement.hpp"
#include "domain/WaferHeightMap.hpp"

namespace Commands
{
  struct DummyMethod 
  {
    std::string waferId;
  };

  struct MeasureWafer 
  {
    std::string waferId;
  };

  using Command = std::variant<DummyMethod, MeasureWafer>;

  class CommandExecutor
  {
    public:
    CommandExecutor(Leveling::Leveling& leveling) : leveling_(leveling){};

    void operator()(const MeasureWafer& cmd)
    {
      leveling_.measureWafer(cmd.waferId);
    }

    void operator()(const DummyMethod& cmd)
    {
      leveling_.dummyMethod(cmd.waferId);
    }

  private:
    Leveling::Leveling& leveling_;
  };
}
