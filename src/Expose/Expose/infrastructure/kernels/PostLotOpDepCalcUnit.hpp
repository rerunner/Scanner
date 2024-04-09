#pragma once

#include <raft>
#include <raftio>

#include "domain/Exposure.hpp"

//FF data for x-n+1 exposures
class PostLotOpDepCalcUnit : public raft::kernel
{
public:
    PostLotOpDepCalcUnit() : kernel()
    {
      input.addPort< Exposure >("inputData");
      output.addPort< Exposure >( "outputPostLOPDepData" );
    }

    virtual ~PostLotOpDepCalcUnit() = default;

    virtual raft::kstatus run()
    {
      Exposure predictionContainer;
      input[ "inputData" ].pop( predictionContainer ); 
      const Exposure outputFFContainer{predictionContainer.GetPrediction()};
      output[ "outputPostLOPDepData" ].push( outputFFContainer );
      return( raft::proceed );
    }

private:
};
