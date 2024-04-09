#pragma once

#include <raft>
#include <raftio>

#include "domain/Exposure.hpp"

//FF data for x-n+1 exposures
class LotOpDependentCalcUnit : public raft::kernel
{
public:
    LotOpDependentCalcUnit() : kernel()
    {
      input.addPort< Exposure >("inputExposureData");
      output.addPort< Exposure >( "outputLOPDepData" );
    }

    virtual ~LotOpDependentCalcUnit() = default;

    virtual raft::kstatus run()
    {
      Exposure predictionContainer;
      input[ "inputExposureData" ].pop( predictionContainer ); 
      const Exposure outputFFContainer{predictionContainer.GetPrediction()};
      output[ "outputLOPDepData" ].push( outputFFContainer );
      return( raft::proceed );
    }

private:
};
