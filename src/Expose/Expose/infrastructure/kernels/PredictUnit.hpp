#pragma once

#include <raft>
#include <raftio>

#include "domain/Exposure.hpp"

//! Prediction data calculations for x-n exposure
class PredictUnit : public raft::kernel
{
public:
  PredictUnit() : kernel()
  {
    output.addPort< Exposure >( "outputPrediction" );
  }

  virtual ~PredictUnit() = default;

  virtual raft::kstatus run()
  {
    float p = 0.0;
    for (int i = 0; i < 15; i++) //! max 15 exposures for one image (demo value)
    {
      std::this_thread::sleep_for (std::chrono::microseconds(1)); //! Imagine one calc to take a 1 microsecond.
      const Exposure out{p};
      output[ "outputPrediction" ].push( out );
    }
    return( raft::stop );
  }

private:
};
