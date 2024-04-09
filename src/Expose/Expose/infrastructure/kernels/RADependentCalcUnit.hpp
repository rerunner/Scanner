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
      input.addPort< Exposure >("inputFFData");
      output.addPort< Exposure >( "outputRADepData" );
    }

    virtual ~RADependentCalcUnit() = default;

    virtual raft::kstatus run()
    {
      Exposure predictionContainer;
      input[ "inputFFData" ].pop( predictionContainer ); 
      const Exposure outputFFContainer{predictionContainer.GetPrediction()};
      output[ "outputRADepData" ].push( outputFFContainer );
      return( raft::proceed );
    }

private:
};
