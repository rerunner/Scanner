#include <iostream>       // std::cout
#include <thread>         // std::thread

#include "Leveling.hpp"
#include "domain/WaferHeightMap.hpp"
#include "infrastructure/base/RepositoryFactory.h"
#include "infrastructure/IWaferHeightMapRepository.hpp"

// Streaming graph
#include <raft>
#include <raftio>
// Streaming Kernel definitions
#include "infrastructure/kernels/PositionSetUnit.hpp"
#include "infrastructure/kernels/MeasureUnit.hpp"


namespace Leveling
{
    void Leveling::SetupDataWriter()
    {
        // Initialize, and create a DomainParticipant

        // Create a publisher for the topic

        // Register the WaferHeightMap type

        // Get QoS to use for the topic, could also use TOPIC_QOS_DEFAULT instead

        // Create a topic for the WaferHeightMap type...

        // Get the default QoS for the Data Writer, could also use DATAWRITER_QOS_DEFAULT

        // Create a DataWriter for the WaferHeightMap topic
    }
    
    void Leveling::Publish()
    {
        // call the write method of the WaferHeightMap datawriter
    }

    void Leveling::measureWafer(std::string waferId)
    {
      std::cout << "measureWafer starts with wafer Id = " << waferId << std::endl;

      //Create Factory for the WaferHeightMap repository
      IRepositoryFactory<WaferHeightMap> *repositoryFactory = new RepositoryFactory<WaferHeightMap>;
      
      //Use factory to create specialized repository to store on Heap Memory or ORM
      //auto *myRepo = repositoryFactory->GetRepository(RepositoryType::HeapRepository);
      auto *myRepo = repositoryFactory->GetRepository(RepositoryType::ORM);

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

#if 0      
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
#endif

      std::cout << "measureWafer done" << std::endl;
    }
}