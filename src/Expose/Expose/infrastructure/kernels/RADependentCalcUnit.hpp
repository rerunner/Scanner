#pragma once

#include <raft>
#include <raftio>

#include "domain/Exposure.hpp"

//FF data for x-n+1 exposures
class RADependentCalcUnit : public raft::kernel
{
public:
    RADependentCalcUnit() : kernel()
    {
      input.addPort< ExposeContext::Exposure >("inputFFData");
      output.addPort< ExposeContext::Exposure >( "outputRADepData" );
    }

    virtual ~RADependentCalcUnit() = default;

    virtual raft::kstatus run()
    {
      ExposeContext::Exposure predictionContainer;
      input[ "inputFFData" ].pop( predictionContainer ); 
      const ExposeContext::Exposure outputFFContainer{predictionContainer.GetPrediction()};
      output[ "outputRADepData" ].push( outputFFContainer );
      return( raft::proceed );
    }

private:
};
