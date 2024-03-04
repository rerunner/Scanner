#pragma once

#include <raft>
#include <raftio>

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
    for (int i = 0; i < 10000; i++)
    {
      const Position out{xpos++, ypos++};
      output[ "outputPosition" ].push( out );
    }
    return( raft::stop );
  }

private:
};
