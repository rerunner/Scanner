#pragma once

#include <raft>
#include <raftio>

#include "domain/Position.hpp"
#include "domain/MarkMeasurement.hpp"

//Measure unit measures the height for every input position
class MeasureUnit : public raft::kernel
{
public:
    MeasureUnit() : kernel()
    {
      input.addPort< Position >("inputPosition");
      output.addPort< MarkMeasurement >( "outputMeasurement" );
    }

    virtual ~MeasureUnit() = default;

    virtual raft::kstatus run()
    {
      Position positionContainer;
      input[ "inputPosition" ].pop( positionContainer ); // Receive position from input port
      double randomZ = static_cast <double> (rand()) / static_cast <double> (RAND_MAX);
      const MarkMeasurement measurementContainer{positionContainer, randomZ}; // Do Measurement
      output[ "outputMeasurement" ].push( measurementContainer ); // Push measurement to output port
      return( raft::proceed );
    }

private:
};
