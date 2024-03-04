#pragma once

#include <raft>
#include <raftio>

#include "domain/Position.hpp"
#include "domain/Measurement.hpp"

//Measure unit measures the height for every input position
class MeasureUnit : public raft::kernel
{
public:
    MeasureUnit() : kernel()
    {
      input.addPort< Position >("inputPosition");
      output.addPort< Measurement >( "outputMeasurement" );
    }

    virtual ~MeasureUnit() = default;

    virtual raft::kstatus run()
    {
      Position positionContainer;
      input[ "inputPosition" ].pop( positionContainer ); // Receive position from input port
      const Measurement measurementContainer{positionContainer, 1.0}; // Do Measurement
      output[ "outputMeasurement" ].push( measurementContainer ); // Push measurement to output port
      return( raft::proceed );
    }

private:
};
