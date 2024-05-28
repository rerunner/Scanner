#pragma once

#include <raft>
#include <raftio>

#include "ScannerC.h"
#include "domain/Position.hpp"

//Source unit generates positions to measure on the wafer.
class PositionSetUnit : public raft::kernel
{
public:
  PositionSetUnit() : kernel()
  {
    output.addPort< Position >( "outputPosition" );
  }

  virtual ~PositionSetUnit() = default;

  virtual raft::kstatus run()
  {
    double xpos = 0.0, ypos = 0.0;
    for (int i = 0; i < scanner::generated::MAX_MEASUREMENT_STEPS; i++)
    {
      std::this_thread::sleep_for (std::chrono::microseconds(1)); // Imagine one positioning action to take 1 microsecond.
      
      auto c(output["outputPosition"].template allocate_s<Position>());
      Position positionContainer{xpos++, ypos++};
      *c = positionContainer;
      output["outputPosition"].send();
    }
    return( raft::stop );
  }

private:
};
