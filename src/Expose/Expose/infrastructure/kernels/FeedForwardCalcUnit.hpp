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
      input.addPort< Exposure >("inputPrediction");
      output.addPort< Exposure >( "outputFFData" );
    }

    virtual ~FeedForwardCalcUnit() = default;

    virtual raft::kstatus run()
    {
      Exposure predictionContainer;
      input[ "inputPrediction" ].pop( predictionContainer ); 
      const Exposure outputFFContainer{predictionContainer.GetPrediction()};
      output[ "outputFFData" ].push( outputFFContainer );
      return( raft::proceed );
    }

private:
};
