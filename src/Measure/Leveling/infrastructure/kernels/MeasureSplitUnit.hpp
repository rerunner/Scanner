#pragma once

#include <raft>
#include <raftio>

#include "domain/Position.hpp"
#include "domain/MarkMeasurement.hpp"

//
//      ┌--- OUT a
// In --|
//      └--- OUT b
//
class MeasureSplitUnit : public raft::kernel
{
public:
    MeasureSplitUnit() : kernel()
    {
      input.addPort< Position >("inputPosition");
      output.addPort< MarkMeasurement >( "outputMeasurement_a", "outputMeasurement_b" );
    }

    virtual ~MeasureSplitUnit() = default;

    virtual raft::kstatus run()
    {
      GSL::Dprintf(GSL::DEBUG, "MeasureSplitUnit kernel run");

      auto &input_port((this)->input["inputPosition"]);
      auto &positionContainer(input_port.template peek<Position>());

      double randomZ = static_cast <double> (rand()) / static_cast <double> (RAND_MAX);
      
      MarkMeasurement measurementContainer_a{positionContainer, randomZ}; // Do Measurement
      MarkMeasurement *measurementContainer_b = &measurementContainer_a;
      
      auto c1(output["outputMeasurement_a"].template allocate_s<MarkMeasurement>());
      auto c2(output["outputMeasurement_b"].template allocate_s<MarkMeasurement>());
      *c1 = measurementContainer_a;
      *c2 = *measurementContainer_b;
      output["outputMeasurement_a"].send();
      output["outputMeasurement_b"].send();
      
      input_port.recycle();

      return( raft::proceed );
    }

private:
};
