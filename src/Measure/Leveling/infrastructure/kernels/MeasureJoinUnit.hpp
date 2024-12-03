#pragma once

#include <raft>
#include <raftio>

#include "domain/MarkMeasurement.hpp"

//Measure unit measures the height for every input position
class MeasureJoinUnit : public raft::kernel
{
public:
    MeasureJoinUnit() : kernel()
    {
      input.addPort< LevelingContext::MarkMeasurement >("input_a", "input_b");
      output.addPort< LevelingContext::MarkMeasurement >( "outputMeasurement" );
    }

    virtual ~MeasureJoinUnit() = default;

    virtual raft::kstatus run()
    {
      auto &input_port_a((this)->input["input_a"]);
      auto &container_a(input_port_a.template peek<LevelingContext::MarkMeasurement>());
      auto &input_port_b((this)->input["input_b"]);
      auto &container_b(input_port_b.template peek<LevelingContext::MarkMeasurement>());

      LevelingContext::Position finalPosition(container_a.GetPosition().GetX(), container_b.GetPosition().GetY());
      double finalZ = (container_a.GetZ() + container_b.GetZ()) / 2.0;

      auto c(output["outputMeasurement"].template allocate_s<LevelingContext::MarkMeasurement>());
      LevelingContext::MarkMeasurement measurementContainer(finalPosition, finalZ);
      *c = measurementContainer;
      output["outputMeasurement"].send();
      input_port_a.recycle();
      input_port_b.recycle();

      return( raft::proceed );
    }

private:
};
