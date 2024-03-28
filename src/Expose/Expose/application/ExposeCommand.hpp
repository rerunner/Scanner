#pragma once

#include <iostream>       // std::cout
#include <thread>         // std::thread
#include <variant>        // std::variant

#include "Expose/application/Expose.hpp"

namespace Expose { namespace Application { namespace ExposeCommands {
  struct DummyMethod 
  {
    std::string waferId;
  };

  struct ExposeWafer 
  {
    std::string waferId;
  };

  using Command = std::variant<DummyMethod, ExposeWafer>;

  class CommandExecutor
  {
    public:
    CommandExecutor(Application::Expose& expose) : expose_(expose){};

    void operator()(const ExposeWafer& cmd)
    {
      expose_.exposeWafer(cmd.waferId);
    }

    void operator()(const DummyMethod& cmd)
    {
      expose_.dummyMethod(cmd.waferId);
    }

  private:
    Application::Expose& expose_;
  };
}}} // namespaces
