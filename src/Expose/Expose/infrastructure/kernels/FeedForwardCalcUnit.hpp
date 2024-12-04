#pragma once

#include <raft>
#include <raftio>

#include "domain/Exposure.hpp"

//FF data for x-n+1 exposures
class FeedForwardCalcUnit : public raft::kernel
{
public:
    FeedForwardCalcUnit() : kernel()
    {
      input.addPort< ExposeContext::Exposure >("inputPrediction");
      output.addPort< ExposeContext::Exposure >( "outputFFData" );
    }

    virtual ~FeedForwardCalcUnit() = default;

    virtual raft::kstatus run()
    {
      ExposeContext::Exposure predictionContainer;
      input[ "inputPrediction" ].pop( predictionContainer ); 
      const ExposeContext::Exposure outputFFContainer{predictionContainer.GetPrediction()};
      output[ "outputFFData" ].push( outputFFContainer );
      return( raft::proceed );
    }

private:
};
