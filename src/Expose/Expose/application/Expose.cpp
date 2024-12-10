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
using namespace Verdi;

namespace Expose { namespace Application 
{
    Expose::Expose(): WAFER_DOMAIN_ID(0), quit_(false)
    {
        eventListenerThread = std::thread(&Expose::eventListenerThreadHandler, this);

        Subscribe();
        StartHeightMapListener();
        
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
        cppkafka::ConfigurationOption exposeConfigOption{"metadata.broker.list", "localhost:9092"};
        kafkaConfigOptions.push_back(exposeConfigOption);
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
            cppkafka::Message record = kafkaConsumer->poll(std::chrono::milliseconds(1000));
            if (record)
            {
                if (!record.get_error())
                {
                    json j_message = json::from_cbor(record.get_payload());
                    GSL::Dprintf(GSL::DEBUG, "Processing NewWaferState message for Wafer Id = ", j_message["Id"], " new wafer state = ", j_message["State"]);
                    GSL::Dprintf(GSL::DEBUG, "For Wafer Id = ", j_message["Id"], " new wafer state = ", j_message["State"]);
                    if (j_message["State"] == "Unloaded")
                    {
                        std::string wIdstr = j_message["Id"];
                        Uuid waferID(wIdstr);
                        GSL::Dprintf(GSL::DEBUG, "Received request to DELETE heightmap for Wafer ID ", waferID.Get());
                        std::unique_ptr<IRepositoryFactory<ExposeContext::WaferHeightMap>> repositoryFactory = std::make_unique<RepositoryFactory<ExposeContext::WaferHeightMap>>();
                        auto repository = repositoryFactory->GetRepository(RepositoryTypeBase::REPOSITORY_TYPE, UoWFactory.GetDataBasePtr());
                        //Uuid waferID(j_message["Id"]);
                        auto whmList = repository->GetAllChildren(waferID); //Fetch all heightmaps in the database and find the one with the correct wafer id
                        GSL::Dprintf(GSL::DEBUG, "Searched for waferheightmap to delete. found number = ", whmList.size());
                        for (int posInList = 0; posInList < whmList.size(); posInList++)
                        {
                            GSL::Dprintf(GSL::DEBUG, "Deleting heightmap for wafer id ", whmList[posInList].GetParentId().Get());
                            repository->Delete(whmList[posInList]); 
                        }
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
        expose_topic_qos.reliability.kind = DDS::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;

        //! Create a topic for the WaferHeightMap type...
        CORBA::String_var type_name = waferheightmap_ts->get_type_name();
        DDS::Topic_var waferheightmap_topic = participant->create_topic ( scanner::generated::WAFER_HEIGHTMAP_TOPIC,
                                                                            type_name,
                                                                            expose_topic_qos, //TOPIC_QOS_DEFAULT,
                                                                            0,
                                                                            ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);


        //! Create the WaferHeightMap DataReader
        DDS::DataReaderQos expose_dr_qos;
        sub->get_default_datareader_qos (expose_dr_qos);
        //expose_dr_qos.durability.kind = DDS::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;
        expose_dr_qos.reliability.kind = DDS::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;

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

    void Expose::StartHeightMapListener()
    {
        GSL::Dprintf(GSL::DEBUG, "Expose fetching matching HeightMap");
        DDS::DataReaderListener_var waferheightmap_listener(new DataReaderListenerImpl(&UoWFactory));
        waferheightmap_dr->ptr()->set_listener(waferheightmap_listener, DDS::DATA_AVAILABLE_STATUS | DDS::LIVELINESS_CHANGED_STATUS);
    }

    void Expose::exposeWafer(Uuid waferID)
    {
        GSL::Dprintf(GSL::DEBUG, "exposeWafer called for wafer Id = ", waferID.Get());
        // Read the stored heightmap
        std::unique_ptr<IRepositoryFactory<ExposeContext::WaferHeightMap>> repositoryFactory = std::make_unique<RepositoryFactory<ExposeContext::WaferHeightMap>>();
        auto repository = repositoryFactory->GetRepository(RepositoryTypeBase::REPOSITORY_TYPE, UoWFactory.GetDataBasePtr());
        auto whmList = repository->GetAllChildren(waferID); //Fetch all heightmaps in the database and find the one with the correct wafer id
        if(whmList.size() > 0)
        {
            //! Start Expose loop
            GSL::Dprintf(GSL::DEBUG, "Start expose loop");
            {
                //! Raft streaming start
                ExposeContext::Exposure generatedExposure;
                PredictUnit predictUnit;
                FeedForwardCalcUnit feedForwardCalcUnit;
                RADependentCalcUnit raDependentCalcUnit;
                LotOpDependentCalcUnit lotOpDependentCalcUnit(&whmList[0]);
                PostLotOpDepCalcUnit postLotOpDepCalcUnit;

                using SinkLambdaExposeResult = raft::lambdak<ExposeContext::Exposure>;
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
        else
        {
            GSL::Dprintf(GSL::ERROR, "No WaferHeightMap found for Expose Loop");
        }
    }
}}
