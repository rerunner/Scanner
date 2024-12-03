#pragma once

#include <raft>
#include <raftio>

#include "domain/MarkMeasurement.hpp"

//Measure unit operates on measurements
class MeasureUnit : public raft::kernel
{
  private:
  std::string name_;
public:
    MeasureUnit(std::string name = "") : kernel()
    {
      input.addPort< LevelingContext::MarkMeasurement >("inputMeasurement");
      output.addPort< LevelingContext::MarkMeasurement >( "outputMeasurement" );
      name_ = name;
    }

    virtual ~MeasureUnit() = default;

    virtual raft::kstatus run()
    {
      auto &input_port((this)->input["inputMeasurement"]);
      auto &a(input_port.template peek<LevelingContext::MarkMeasurement>());

      // FIXME How to pass the input memory to the output ?
      auto c(output["outputMeasurement"].template allocate_s<LevelingContext::MarkMeasurement>());
      (*c) = a;
      output["outputMeasurement"].send();
      input_port.recycle();
      //input_port.unpeek(); // We don't really need to unpeek here do we ?

        return (raft::proceed);
    }

private:
};
