#include <iostream>       // std::cout
#include <thread>         // std::thread

#include "Leveling.hpp"

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
#include "dds/DCPS/transport/rtps_udp/RtpsUdp.h"
#include "dds/DCPS/transport/rtps_udp/RtpsUdpInst.h"
#include "dds/DCPS/transport/rtps_udp/RtpsUdpInst_rch.h"
#include "dds/DCPS/RcHandle_T.h"
#include "dds/DCPS/RTPS/RtpsDiscovery.h"
#include <dds/DCPS/transport/tcp/TcpInst.h>

#include <nlohmann/json.hpp>

#include "ScannerC.h"
#include "GenLogger.hpp"
#include "domain/WaferHeightMap.hpp"

using json = nlohmann::json;

namespace Leveling  { namespace Application
{
    Leveling::Leveling(): WAFER_DOMAIN_ID(0)
    {
      eventListenerThread = std::thread(&Leveling::eventListenerThreadHandler, this);
      GSL::Dprintf(GSL::DEBUG, "Leveling constructed with DDS WAFER_DOMAIN_ID ", WAFER_DOMAIN_ID);
    }

    Leveling::~Leveling()
    {
      quit_ = true;
    }

    void Leveling::eventListenerThreadHandler()
    {
        // Create the Kafka config
        GSL::Dprintf(GSL::DEBUG, "Creating the Kafka config");
        std::vector<cppkafka::ConfigurationOption> kafkaConfigOptions;
        cppkafka::ConfigurationOption machinecontrolConfigOption{"metadata.broker.list", "localhost:9092"};
        kafkaConfigOptions.push_back(machinecontrolConfigOption);
        kafkaConfigOptions.push_back({ "group.id", "leveling" }); // Every microservice needs its own unique kafka group id
        kafkaConfig = std::make_unique<cppkafka::Configuration>(cppkafka::Configuration{kafkaConfigOptions});
        
        // Create a consumer instance
        GSL::Dprintf(GSL::DEBUG, "Creating a consumer instance");
        kafkaConsumer = std::make_unique<cppkafka::Consumer>(*kafkaConfig);

        // Subscribe to topics
        std::vector<std::string> levelingTopics;
        levelingTopics.push_back("waferStateTopic");
        GSL::Dprintf(GSL::DEBUG, "Subscribing to Lot and Wafer topics");
        kafkaConsumer->subscribe(levelingTopics);
        do
        {
            // Poll messages from Kafka brokers
            cppkafka::Message record = kafkaConsumer->poll(std::chrono::milliseconds(100));
            if (record)
            {
                if (!record.get_error())
                {
                    std::ostringstream newMessageStream;
                    newMessageStream << record.get_payload();
                    std::string newMessage = newMessageStream.str();
                    GSL::Dprintf(GSL::DEBUG, "Got a new message...", newMessage);

                    GSL::Dprintf(GSL::DEBUG, "processing NewWaferState message");
                    json j_message = json::from_cbor(record.get_payload());
                    GSL::Dprintf(GSL::DEBUG, "For Wafer Id = ", j_message["Id"], " new wafer state = ", j_message["State"]);
                    if (j_message["State"] == "Unloaded")
                    {
                        GSL::Dprintf(GSL::ERROR, "CAN DELETE WAFER with ID ", j_message["Id"]);
                        UnitOfWork context_;
                        GSL::Dprintf(GSL::DEBUG, "UoW created");
                        auto repository = context_.GetRepository<WaferHeightMap>();
                        GSL::Dprintf(GSL::DEBUG, "repository opened to waferheightmap");
                        auto whmList = repository->GetAll();
                        GSL::Dprintf(GSL::DEBUG, "repository requested to return all wafer heightmaps");
                        Uuid targetId(j_message["Id"]);
                        GSL::Dprintf(GSL::DEBUG, "Searching for wafer id in list of wafer heightmaps");
                        for (auto &iter:whmList)
                        {
                          GSL::Dprintf(GSL::DEBUG, "Searching in wafer heightmap id ", iter.GetId().Get(), " with wID = ", iter.GetWaferId().Get());
                          if (targetId.Get() == iter.GetWaferId().Get())
                          {
                            GSL::Dprintf(GSL::DEBUG, "MATCH FOUND !!!!! Deleting WaferHeightMap of Wafer Id", targetId.Get());
                            repository->Delete(iter);
                          }
                        }
                    }
                    
                }
                else if (!record.is_eof()) {
                    // Is it an error notification, handle it.
                    // This is explicitly skipping EOF notifications as they're not actually errors,
                    // but that's how rdkafka provides them
                    GSL::Dprintf(GSL::ERROR, "Leveling kafka error");    
                }
            }
        } while(!quit_);
    }

    void Leveling::SetupDataWriter()
    {
        // Initialize, and create a DomainParticipant
        int argc = 7; 
        char* argv[] = {const_cast<char *>("./Scanner"), 
                        const_cast<char *>("-ORBDebugLevel"), 
                        const_cast<char *>("10"), 
                        const_cast<char *>("-DCPSDebugLevel"), 
                        const_cast<char *>("10"),
                        const_cast<char *>("-ORBLogFile"),
                        const_cast<char *>("LevelingPublisher.log")
                        };
        
        dpf = std::make_unique<DDS::DomainParticipantFactory_var>(TheParticipantFactoryWithArgs(argc, argv));
        
        participant = std::make_unique<DDS::DomainParticipant_var>(dpf.get()->ptr()->create_participant(  WAFER_DOMAIN_ID,
                                                                    PARTICIPANT_QOS_DEFAULT,
                                                                    0,  // No listener required
                                                                    ::OpenDDS::DCPS::DEFAULT_STATUS_MASK));
        if (!participant) 
	      {
          std::stringstream errss;
          errss << "Failed to create DomainParticipant object";
          GSL::Dprintf(GSL::FATAL, errss.str());
          throw errss.str();
	      }

        // Register the WaferHeightMap type
        scanner::generated::WaferHeightMapTypeSupport_var waferheightmap_servant = new scanner::generated::WaferHeightMapTypeSupportImpl();
        if (DDS::RETCODE_OK != waferheightmap_servant->register_type(participant.get()->ptr(), "")) 
        {
          std::stringstream errss;
          std::cerr << "register_type failed." << std::endl;
          GSL::Dprintf(GSL::FATAL, errss.str());
          throw errss.str();
        }

        // Get QoS to use for the topic, could also use TOPIC_QOS_DEFAULT instead
        DDS::TopicQos leveling_topic_qos;
        participant.get()->ptr()->get_default_topic_qos(leveling_topic_qos);
        leveling_topic_qos.durability.kind = DDS::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;

        // Create a topic for the WaferHeightMap type...
        DDS::Topic_var waferheightmap_topic = participant.get()->ptr()->create_topic ( scanner::generated::WAFER_HEIGHTMAP_TOPIC,
                                                                          waferheightmap_servant->get_type_name (),
                                                                          leveling_topic_qos, //TOPIC_QOS_DEFAULT,
                                                                          0,
                                                                          ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);

        // Create a publisher for the topic
        pub = participant.get()->ptr()->create_publisher(PUBLISHER_QOS_DEFAULT,
                                            0,
                                            ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);

        //DDS::TopicDescription_ptr const topic_description = participant->lookup_topicdescription(scanner::generated::WAFER_HEIGHTMAP_TOPIC);

        DDS::DataWriterQos leveling_dr_qos;
        pub->get_default_datawriter_qos (leveling_dr_qos);
        leveling_dr_qos.durability.kind = DDS::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;
        // Create a DataWriter for the WaferHeightMap topic
        waferHeightMap_base_dw = pub->create_datawriter(  waferheightmap_topic,
                                                          leveling_dr_qos, //DATAWRITER_QOS_DEFAULT,
                                                          0,
                                                          ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);

        // Safely downcast data writer to type-specific data writer
        //waferHeightMap_dw = scanner::generated::WaferHeightMapDataWriter::_narrow(waferHeightMap_base_dw.in());
        waferHeightMap_dw = scanner::generated::WaferHeightMapDataWriter::_narrow(waferHeightMap_base_dw);

        GSL::Dprintf(GSL::DEBUG, "Leveling datawriter created, subscribed to topic ", waferheightmap_topic->get_name());

        // Register the WaferHeightMap
        whm_handle = waferHeightMap_dw->register_instance(whm_evt);
    }
    
    DDS::ReturnCode_t Leveling::Publish(UnitOfWork *passedWhmContext, std::shared_ptr<WaferHeightMap> waferHeightMap)
    {
      // Get the heightmap to publish
      std::list<Measurement> myHeightMap = waferHeightMap->GetHeightMap();

      // DTO assembler start
      scanner::generated::WaferHeightMap newWaferHeightMapDTO;
      newWaferHeightMapDTO.heightMapID = waferHeightMap->GetId().Get().c_str();
      newWaferHeightMapDTO.waferID = waferHeightMap->GetWaferId().Get().c_str();
      newWaferHeightMapDTO.measurements.length(10000); // Looping through all of the elements:
      for (int i = 0; Measurement myMeas : myHeightMap) //C++20 syntax
      {
        Position myPosition = myMeas.GetPosition();
        newWaferHeightMapDTO.measurements[i].xyPosition.xPos = myPosition.GetX(); 
        newWaferHeightMapDTO.measurements[i].xyPosition.yPos = myPosition.GetY(); 
        newWaferHeightMapDTO.measurements[i].zPos = myMeas.GetZ();
        i++;
      }
      // DTO assembler end

      // call the write method of the WaferHeightMap datawriter
      GSL::Dprintf(GSL::DEBUG, "Publishing HeightMap of WAFER ID = ", newWaferHeightMapDTO.waferID, " using DDS datawriter.");
      return waferHeightMap_dw->write(newWaferHeightMapDTO, whm_handle);
    }

    Uuid Leveling::measureWafer(Uuid waferId)
    {
      UnitOfWork context_;

      GSL::Dprintf(GSL::DEBUG, "measureWafer starts with wafer Id = ", waferId.Get());

      // Create empty wafer heightmap
      std::shared_ptr<WaferHeightMap> waferHeightMap = std::make_shared<WaferHeightMap>(waferId);
      GSL::Dprintf(GSL::DEBUG, "WaferHeightMap created with heightmap ID = ", waferHeightMap->GetId().Get());
      
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
            waferHeightMap->AddMeasurement(generatedMeasurement); // And add it to the heightmap
            return( raft::proceed ); // Wait for the next measurement or stream end tag.
          });

      raft::map m;
      m += positionSetUnit >> measureUnit >> sinkLambda;
      m.exe();
      //Raft streaming End
      
      context_.RegisterNew<WaferHeightMap>(waferHeightMap);
      
      GSL::Dprintf(GSL::DEBUG, "WaferHeightMap with ID = ", waferHeightMap->GetId().Get(), " persisted");

      this->SetupDataWriter();
      this->Publish(&context_, waferHeightMap);
      GSL::Dprintf(GSL::DEBUG, "measureWafer done");
      context_.Commit(); //Use case "measure height map" ended
      return waferHeightMap->GetId();
    }
}}