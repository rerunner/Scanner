#pragma once

#include <raft>
#include <raftio>

#include "domain/Exposure.hpp"
#include "domain/WaferHeightMap.hpp"

//FF data for x-n+1 exposures
class LotOpDependentCalcUnit : public raft::kernel
{
private:
    LevelingContext::WaferHeightMap *whm;
public:
    LotOpDependentCalcUnit(LevelingContext::WaferHeightMap *receivedWhm) : kernel()
    {
      input.addPort< Exposure >("inputExposureData");
      output.addPort< Exposure >( "outputLOPDepData" );
      whm = receivedWhm;
    }

    virtual ~LotOpDependentCalcUnit() = default;

    virtual raft::kstatus run()
    {
      Exposure predictionContainer;
      input[ "inputExposureData" ].pop( predictionContainer ); 
      const Exposure outputFFContainer{predictionContainer.GetPrediction()};
      output[ "outputLOPDepData" ].push( outputFFContainer );
      GSL::Dprintf(GSL::DEBUG, "Raft kernel received waferheightmap ", whm->GetId().Get());
      return( raft::proceed );
    }

private:
};
