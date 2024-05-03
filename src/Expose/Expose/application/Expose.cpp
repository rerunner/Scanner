#include "Expose/application/Expose.hpp"

// Streaming graph
#include <raft>
#include <raftio>
// Streaming Kernel definitions
#include "infrastructure/kernels/PredictUnit.hpp"
#include "infrastructure/kernels/FeedForwardCalcUnit.hpp"
#include "infrastructure/kernels/RADependentCalcUnit.hpp"
#include "infrastructure/kernels/LotOpDependentCalcUnit.hpp"
#include "infrastructure/kernels/PostLotOpDepCalcUnit.hpp"

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

#include <dds/DdsDcpsSubscriptionS.h>
#include <dds/DCPS/LocalObject.h>
#include <dds/DCPS/Definitions.h>

#include <nlohmann/json.hpp>

#include "ScannerC.h"
#include "GenLogger.hpp"

using json = nlohmann::json;

namespace Expose { namespace Application 
{
    Expose::Expose(): WAFER_DOMAIN_ID(0), quit_(false)
    {
        eventListenerThread = std::thread(&Expose::eventListenerThreadHandler, this);
        GSL::Dprintf(GSL::DEBUG, "Expose object created");
    }
    Expose::~Expose()
    {
        quit_ = true;
        this->UnSubscribe();
    }

    void Expose::eventListenerThreadHandler()
    {
        // Create the Kafka config
        GSL::Dprintf(GSL::DEBUG, "Creating the Kafka config");
        std::vector<cppkafka::ConfigurationOption> kafkaConfigOptions;
        cppkafka::ConfigurationOption machinecontrolConfigOption{"metadata.broker.list", "localhost:9092"};
        kafkaConfigOptions.push_back(machinecontrolConfigOption);
        kafkaConfigOptions.push_back({ "group.id", "expose" }); // Every microservice needs its own unique kafka group id
        kafkaConfig = std::make_unique<cppkafka::Configuration>(cppkafka::Configuration{kafkaConfigOptions});
        
        // Create a consumer instance
        GSL::Dprintf(GSL::DEBUG, "Creating a consumer instance");
        kafkaConsumer = std::make_unique<cppkafka::Consumer>(*kafkaConfig);

        // Subscribe to topics
        std::vector<std::string> exposeTopics;
        exposeTopics.push_back("waferStateTopic");
        GSL::Dprintf(GSL::DEBUG, "Subscribing to Lot and Wafer topics");
        kafkaConsumer->subscribe(exposeTopics);
        do
        {
            // Poll messages from Kafka brokers
            cppkafka::Message record = kafkaConsumer->poll(std::chrono::milliseconds(100));
            if (record)
            {
                if (!record.get_error())
                {
                    json j_message = json::from_cbor(record.get_payload());
                    GSL::Dprintf(GSL::DEBUG, "Processing NewWaferState message for Wafer Id = ", j_message["Id"], " new wafer state = ", j_message["State"]);
                    GSL::Dprintf(GSL::DEBUG, "For Wafer Id = ", j_message["Id"], " new wafer state = ", j_message["State"]);
                    if (j_message["State"] == "Unloaded")
                    {
                        GSL::Dprintf(GSL::DEBUG, "Can delete heightmap data associated with Wafer ID ", j_message["Id"]);
                        std::unique_ptr<IRepositoryFactory<WaferHeightMap>> repositoryFactory = std::make_unique<RepositoryFactory<WaferHeightMap>>();
                        auto repository = repositoryFactory->GetRepository(RepositoryType::ORM);
                        auto whmList = repository->GetAll();
                        Uuid targetId(j_message["Id"]);
                        for (auto &iter:whmList)
                        {
                          GSL::Dprintf(GSL::DEBUG, "Searching WaferHeightMap List, found Wafer Id = ", iter.GetWaferId().Get());
                          if (targetId.Get() == iter.GetWaferId().Get())
                          {
                            GSL::Dprintf(GSL::DEBUG, "MATCH FOUND !!!!! Deleting WaferHeightMap of Wafer Id = ", targetId.Get());
                            repository->Delete(iter);
                          }
                        }
                        GSL::Dprintf(GSL::DEBUG, "Searching WaferHeightMap List DONE!");
                    }
                    
                }
                else if (!record.is_eof()) {
                    // Is it an error notification, handle it.
                    // This is explicitly skipping EOF notifications as they're not actually errors,
                    // but that's how rdkafka provides them
                    GSL::Dprintf(GSL::ERROR, "Expose kafka error");    
                }
            }
        } while(!quit_);
    }

    void Expose::Subscribe()
    {
        //! Initialize DomainParticipantFactory
        int argc = 9; 
        char* argv[] = {const_cast<char *>("./Expose"), 
                        const_cast<char *>("-ORBDebugLevel"), 
                        const_cast<char *>("10"), 
                        const_cast<char *>("-DCPSDebugLevel"), 
                        const_cast<char *>("10"),
                        const_cast<char *>("-DCPSTransportDebugLevel"),
                        const_cast<char *>("6"),
                        const_cast<char *>("-ORBLogFile"),
                        const_cast<char *>("ExposeSubscriber.log")
                        };
        DDS::DomainParticipantFactory_var dpf = DDS::DomainParticipantFactory::_nil();
        DDS::DomainParticipant_var participant = DDS::DomainParticipant::_nil();

        dpf = TheParticipantFactoryWithArgs(argc, argv);
        
        //! Create DomainParticipant
        participant = dpf->create_participant(  WAFER_DOMAIN_ID,
                                                PARTICIPANT_QOS_DEFAULT,
                                                0,  // No listener required
                                                ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);

        if (!participant)
        {
            std::stringstream errss;
            std::cerr << "participant failed. Perhaps DCPS repo not running?" << std::endl;
            GSL::Dprintf(GSL::FATAL, errss.str()); 
            throw errss.str();
        }

        //! Create a subscriber for the topic
        DDS::Subscriber_var sub = participant->create_subscriber(   SUBSCRIBER_QOS_DEFAULT,
                                                                    0,
                                                                    ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);

        //! Register the WaferHeightMap type
        scanner::generated::WaferHeightMapTypeSupport_var waferheightmap_ts = new scanner::generated::WaferHeightMapTypeSupportImpl();
        
        if (DDS::RETCODE_OK != waferheightmap_ts->register_type(participant.in (), waferheightmap_ts->get_type_name()))
        {
            std::stringstream errss;
            std::cerr << "register_type failed." << std::endl;
            GSL::Dprintf(GSL::FATAL, errss.str()); 
            throw errss.str();
        }

        //! Get QoS to use for the topic, could also use TOPIC_QOS_DEFAULT instead
        DDS::TopicQos expose_topic_qos;
        participant->get_default_topic_qos(expose_topic_qos);
        expose_topic_qos.durability.kind = DDS::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;

        //! Create a topic for the WaferHeightMap type...
        CORBA::String_var type_name = waferheightmap_ts->get_type_name();
        DDS::Topic_var waferheightmap_topic = participant->create_topic ( scanner::generated::WAFER_HEIGHTMAP_TOPIC,
                                                                            type_name,
                                                                            expose_topic_qos,
                                                                            0,
                                                                            ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);


        //! Create the WaferHeightMap DataReader
        DDS::DataReaderQos expose_dr_qos;
        sub->get_default_datareader_qos (expose_dr_qos);
        expose_dr_qos.durability.kind = DDS::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;

        waferheightmap_dr = new DDS::DataReader_var(sub->create_datareader(  waferheightmap_topic,
                                                                expose_dr_qos, 
                                                                NULL, // will be set to waferheightmap_listener in StartHeightMapListener()
                                                                OpenDDS::DCPS::NO_STATUS_MASK)); 
        if (!waferheightmap_dr)
        {
            std::stringstream errss;
            std::cerr << "waferheightmap_dr failed." << std::endl;
            GSL::Dprintf(GSL::FATAL, errss.str()); 
            throw errss.str();
        }
        
    }

    void Expose::UnSubscribe()
    {
        if (TheServiceParticipant)
        {
            TheServiceParticipant->shutdown ();
        }
    }

    std::string Expose::StartHeightMapListener(std::unique_ptr<UnitOfWork> & context_)
    {
        GSL::Dprintf(GSL::DEBUG, "Looking for matching HeightMap");

        //! Creating the infrastructure that allows an application thread to block
        //! until some condition becomes true, such as data availability.
        //! Create a Status Condition for the reader
        //! Create DataReaderListeners 
        std::promise<std::string> theHeightMapId;
        std::future<std::string> f = theHeightMapId.get_future();
        
        DDS::DataReaderListener_var waferheightmap_listener(new DataReaderListenerImpl(context_, &theHeightMapId));
        waferheightmap_dr->ptr()->set_listener(waferheightmap_listener, DDS::DATA_AVAILABLE_STATUS | DDS::LIVELINESS_CHANGED_STATUS);
        std::string ret = f.get(); //! Wait for the future ;-)
        waferheightmap_dr->ptr()->set_listener(0, OpenDDS::DCPS::NO_STATUS_MASK);
        GSL::Dprintf(GSL::DEBUG, "HeightMap, Start committing to database.");
        context_->Commit(); // Write changes to repository
        GSL::Dprintf(GSL::DEBUG, "HeightMap, End committing to database.");
        return ret;
    }

    void Expose::exposeWafer(Uuid waferID)
    {
        GSL::Dprintf(GSL::DEBUG, "exposeWafer starts with wafer Id = ", waferID.Get());
        std::unique_ptr<UnitOfWork> context_ = std::make_unique<UnitOfWork>(); // smart pointer of UoW as this is going to be passed around

        //! expose the whole wafer die by die with the provided image
        //! Uses the wafer heightmap for lens correction.
        //! Two phases repeat: stepping phase (to the next die) and scanning phase (of one die)
        this->Subscribe();
        std::string foundHeightMapId = StartHeightMapListener(context_);

        //! Start Expose loop
        GSL::Dprintf(GSL::DEBUG, "Start expose loop");
        {
            //! Raft streaming start
            Exposure generatedExposure;
            PredictUnit predictUnit;
            FeedForwardCalcUnit feedForwardCalcUnit;
            RADependentCalcUnit raDependentCalcUnit;
            LotOpDependentCalcUnit lotOpDependentCalcUnit;
            PostLotOpDepCalcUnit postLotOpDepCalcUnit;

            using SinkLambdaExposeResult = raft::lambdak<Exposure>;
            SinkLambdaExposeResult sinkLambdaExposeResult(1,/** input port */
                        0, /** output port */
                [&](Port &input,
                    Port &output)
                {
                    UNUSED( output );
                    input[ "0" ].pop( generatedExposure ); //! Take the measurement from the input
                    GSL::Dprintf(GSL::DEBUG, "Expose Loop finished, Exposure ID = ", generatedExposure.GetId().Get());
                    return( raft::proceed ); //! The source will push the stop tag.
                });

            raft::map m;
            m += predictUnit >> feedForwardCalcUnit >> raDependentCalcUnit >> lotOpDependentCalcUnit >> postLotOpDepCalcUnit >> sinkLambdaExposeResult;
            m.exe();
            //! Raft streaming End
        }
        //! End Expose Loop
        GSL::Dprintf(GSL::DEBUG, "End expose loop");
    }
}}