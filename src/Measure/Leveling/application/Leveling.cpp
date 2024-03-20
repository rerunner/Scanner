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

#include "dds/DCPS/Marked_Default_Qos.h"
#include <dds/DCPS/Service_Participant.h> 
#include "ScannerTypeSupportImpl.h"

#include "dds/DCPS/RTPS/RtpsDiscovery.h"
#include "dds/DCPS/transport/framework/TransportRegistry.h"
#include "dds/DCPS/transport/framework/TransportConfig_rch.h"
#include "dds/DCPS/transport/framework/TransportInst.h"
#include "dds/DCPS/StaticIncludes.h"
//#ifdef ACE_AS_STATIC_LIBS
#include "dds/DCPS/transport/rtps_udp/RtpsUdp.h"
//#endif
#include "dds/DCPS/transport/rtps_udp/RtpsUdpInst.h"
#include "dds/DCPS/transport/rtps_udp/RtpsUdpInst_rch.h"

#include "dds/DCPS/RcHandle_T.h"
#include "dds/DCPS/RTPS/RtpsDiscovery.h"

#include <dds/DCPS/transport/tcp/TcpInst.h>
#include "ScannerC.h"

namespace Leveling
{
    Leveling::Leveling(): WAFER_DOMAIN_ID(0)
    {
      std::cout << "WAFER_DOMAIN_ID " << WAFER_DOMAIN_ID << std::endl;
    }

    void Leveling::SetupDataWriter()
    {
        // Initialize, and create a DomainParticipant
        int argc = 5; 
        char* argv[] = {"./Scanner", "-DCPSInfoRepo", "127.0.0.1:12345", "-DCPSConfigFile", "rtps.ini"};
        DDS::DomainParticipantFactory_var dpf = DDS::DomainParticipantFactory::_nil();
        DDS::DomainParticipant_var participant = DDS::DomainParticipant::_nil();

        dpf = TheParticipantFactoryWithArgs(argc, argv);
        //WAFER_DOMAIN_ID = 0;
        participant = dpf->create_participant(  WAFER_DOMAIN_ID,
                                                PARTICIPANT_QOS_DEFAULT,
                                                0,  // No listener required DDS::DomainParticipantListener::_nil(),
                                                ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);
        if (!participant) 
	      {
          std::stringstream errss;
          errss << "Failed to create DomainParticipant object";
          throw errss.str();
	      }

        // Register the WaferHeightMap type
        scanner::generated::WaferHeightMapTypeSupport_var waferheightmap_servant = new scanner::generated::WaferHeightMapTypeSupportImpl();
        if (DDS::RETCODE_OK != waferheightmap_servant->register_type(participant, "")) 
        {
          std::stringstream errss;
          std::cerr << "register_type failed." << std::endl;
          throw errss.str();
        }

        // Get QoS to use for the topic, could also use TOPIC_QOS_DEFAULT instead
        DDS::TopicQos default_topic_qos;
        participant->get_default_topic_qos(default_topic_qos);

        // Create a topic for the WaferHeightMap type...
        DDS::Topic_var waferheightmap_topic = participant->create_topic ( scanner::generated::WAFER_HEIGHTMAP_TOPIC,
                                                                          waferheightmap_servant->get_type_name (),
                                                                          default_topic_qos,
                                                                          DDS::TopicListener::_nil(),
                                                                          ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);
        if (!waferheightmap_topic) 
	      {
          std::stringstream errss;
          errss << "Failed to create waferheightmap_topic.";
          throw errss.str();
	      }

        // Create a publisher for the topic
        pub = participant->create_publisher(PUBLISHER_QOS_DEFAULT,
                                            DDS::PublisherListener::_nil(),
                                            ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);
        if (!pub.in()) 
	      {
          std::stringstream errss;
          errss << "Failed to create publisher.";
          throw errss.str();
	      }

        DDS::TopicDescription_ptr const topic_description = participant->lookup_topicdescription(scanner::generated::WAFER_HEIGHTMAP_TOPIC);

        if (topic_description == nullptr) 
	      {
          std::cout << "troubles ahead?" << std::endl;
        }

        // Get the default QoS for the Data Writer, could also use DATAWRITER_QOS_DEFAULT
        DDS::DataWriterQos dw_default_qos;
        pub->get_default_datawriter_qos (dw_default_qos);

        // Create a DataWriter for the WaferHeightMap topic
        waferHeightMap_base_dw = pub->create_datawriter(  waferheightmap_topic.in (),
                                                          dw_default_qos,
                                                          DDS::DataWriterListener::_nil(),
                                                          ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);
        waferHeightMap_dw = scanner::generated::WaferHeightMapDataWriter::_narrow(waferHeightMap_base_dw.in());

        // Register the WaferHeightMap
        whm_handle = waferHeightMap_dw->register_instance(whm_evt);
    }
    
    void Leveling::Publish()
    {
        // call the write method of the WaferHeightMap datawriter
        // Publish
        scanner::generated::WaferHeightMap newWaferHeightMap;
        newWaferHeightMap.waferID = "1";
        std::cout << "Publishing WAFER 1 using DDS" << std::endl;
        DDS::ReturnCode_t ret = waferHeightMap_dw->write(newWaferHeightMap, whm_handle);
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

      this->SetupDataWriter();
      this->Publish();
      std::cout << "measureWafer done" << std::endl;
    }
}