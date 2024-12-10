#pragma once

#include <raft>
#include <raftio>

#include "domain/Exposure.hpp"

//FF data for x-n+1 exposures
class LotOpDependentCalcUnit : public raft::kernel
{
private:
    ExposeContext::WaferHeightMap *whm;
public:
    LotOpDependentCalcUnit(ExposeContext::WaferHeightMap *receivedWhm) : kernel()
    {
      input.addPort< ExposeContext::Exposure >("inputExposureData");
      output.addPort< ExposeContext::Exposure >( "outputLOPDepData" );
      whm = receivedWhm;
    }

    virtual ~LotOpDependentCalcUnit() = default;

    virtual raft::kstatus run()
    {
      ExposeContext::Exposure predictionContainer;
      input[ "inputExposureData" ].pop( predictionContainer ); 
      const ExposeContext::Exposure outputFFContainer{predictionContainer.GetPrediction()};
      output[ "outputLOPDepData" ].push( outputFFContainer );
      GSL::Dprintf(GSL::DEBUG, "Raft kernel received waferheightmap ", whm->GetId().Get());
      return( raft::proceed );
    }

private:
};
