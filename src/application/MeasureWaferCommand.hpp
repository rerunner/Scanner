#pragma once
#include <iostream>       // std::cout
#include <thread>         // std::thread

#include "infrastructure/base/RepositoryFactory.h"

#include "domain/Position.hpp"
#include "domain/Measurement.hpp"
#include "domain/WaferHeightMap.hpp"
#include "infrastructure/IWaferHeightMapRepository.hpp"
// Streaming graph
#include <raft>
#include <raftio>
// Streaming Kernel definitions
#include "infrastructure/kernels/PositionSetUnit.hpp"
#include "infrastructure/kernels/MeasureUnit.hpp"

namespace Commands
{
  void measureWaferCommandTask(std::string waferId)
  {
    std::cout << "measureWaferCommandTask received with wafer Id = " << waferId << std::endl;

    //Create Factory for the WaferHeightMap repository
    IRepositoryFactory<WaferHeightMap> *repositoryFactory = new RepositoryFactory<WaferHeightMap>;
    
    //Use factory to create specialized repository to store on Heap Memory or ORM
    auto *myRepo = repositoryFactory->GetRepository(RepositoryType::HeapRepository);
    //auto *myRepo = repositoryFactory->GetRepository(RepositoryType::ORM);

    // Create empty wafer heightmap
    WaferHeightMap waferHeightMap;
    std::cout << "WaferHeightMap created with ID = " << waferHeightMap.GetId() << "\n";
    
    // Raft streaming start
    Measurement generatedMeasurement;
    PositionSetUnit positionSetUnit;
    MeasureUnit measureUnit;

    using SinkLambda = raft::lambdak<Measurement>;
    SinkLambda sinkLambda(1,/* input port */
		          0, /* output port */
			  [&](Port &input,
			      Port &output)
			  {
			    UNUSED( output );
			    input[ "0" ].pop( generatedMeasurement ); // Take the measurement from the input
			    waferHeightMap.AddMeasurement(generatedMeasurement); // And add it to the heightmap
			    return( raft::proceed ); // Wait for the next measurement or stream end tag.
			  });

    raft::map m;
    m += positionSetUnit >> measureUnit >> sinkLambda;
    m.exe();
    //Raft streaming End
    
    myRepo->Store(waferHeightMap); //Use case "measure height map" ended
    
    std::cout << "WaferHeightMap with ID = " << waferHeightMap.GetId() << " persisted.\n";

    WaferHeightMap whm_clone = myRepo->Get(waferHeightMap.GetId());
    std::cout << "WaferHeightMap clone created with ID = " << whm_clone.GetId() << "\n";
    
    std::list<Measurement> myHeightMap = whm_clone.GetHeightMap();
    // Looping through all of the elements:
    for (Measurement myMeas : myHeightMap)
    {
      Position myPosition = myMeas.GetPosition();
      std::cout << " Measurement X = " << myPosition.GetX();
      std::cout << " Measurement Y = " << myPosition.GetY();
      std::cout << " Measurement Z = " << myMeas.GetZ() << std::endl;
    }
	
    myRepo->Delete(waferHeightMap);

    std::cout << "trying again";
    myRepo->Delete(waferHeightMap);
    std::cout << " should do nothing" << std::endl;


    std::cout << "measureWaferCommandTask done" << std::endl;
  }
  
  class MeasureWaferCommand
  {
  public:
    MeasureWaferCommand(std::string waferId)
    {
      // Pass this on to a task to perform in the background and return immediately
      std::thread (measureWaferCommandTask, waferId).detach(); //detach thread (until jthreads?)
    }
  };
}
