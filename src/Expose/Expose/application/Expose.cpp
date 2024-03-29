#include "Expose/application/Expose.hpp"
#include "DataReaderListenerImpl.h"

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

#include "ScannerC.h"

namespace Expose { namespace Application 
{
    Expose::Expose(): WAFER_DOMAIN_ID(0)
    {
        std::cout << "Expose object created" << std::endl;
        //Create Factory for the WaferHeightMap repository
        repositoryFactory = new RepositoryFactory<WaferHeightMap>;
        //Use factory to create specialized repository to store on Heap Memory or ORM
        //auto *myRepo = repositoryFactory->GetRepository(RepositoryType::HeapRepository);
        myRepo = repositoryFactory->GetRepository(RepositoryType::ORM);
        this->Subscribe();
    }
    Expose::~Expose()
    {
        this->UnSubscribe();
    }

    void Expose::Subscribe()
    {
        // Initialize DomainParticipantFactory
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
        
        // Create DomainParticipant
        participant = dpf->create_participant(  WAFER_DOMAIN_ID,
                                                PARTICIPANT_QOS_DEFAULT,
                                                0,  // No listener required
                                                ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);

        if (!participant)
        {
            std::stringstream errss;
            std::cerr << "participant failed. Perhaps DCPS repo not running?" << std::endl;
            throw errss.str();
        }

        // Create a subscriber for the topic
        DDS::Subscriber_var sub = participant->create_subscriber(   SUBSCRIBER_QOS_DEFAULT,
                                                                    0,
                                                                    ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);

        // Register the WaferHeightMap type
        scanner::generated::WaferHeightMapTypeSupport_var waferheightmap_ts = new scanner::generated::WaferHeightMapTypeSupportImpl();
        
        if (DDS::RETCODE_OK != waferheightmap_ts->register_type(participant.in (), waferheightmap_ts->get_type_name()))
        {
            std::stringstream errss;
            std::cerr << "register_type failed." << std::endl;
            throw errss.str();
        }

        // Get QoS to use for the topic, could also use TOPIC_QOS_DEFAULT instead
        DDS::TopicQos default_topic_qos;
        participant->get_default_topic_qos(default_topic_qos);

        // Create a topic for the WaferHeightMap type...
        CORBA::String_var type_name = waferheightmap_ts->get_type_name();
        DDS::Topic_var waferheightmap_topic = participant->create_topic ( scanner::generated::WAFER_HEIGHTMAP_TOPIC,
                                                                            type_name,
                                                                            TOPIC_QOS_DEFAULT,
                                                                            0,
                                                                            ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);


        // Create the WaferHeightMap DataReader
        DDS::DataReaderQos dr_default_qos;
        sub->get_default_datareader_qos (dr_default_qos);

        waferheightmap_dr = new DDS::DataReader_var(sub->create_datareader(  waferheightmap_topic,
                                                                DATAREADER_QOS_DEFAULT,
                                                                NULL, // will be set to waferheightmap_listener 
                                                                OpenDDS::DCPS::NO_STATUS_MASK)); 
        if (!waferheightmap_dr)
        {
            std::stringstream errss;
            std::cerr << "waferheightmap_dr failed." << std::endl;
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

    std::string Expose::StartHeightMapListener()
    {
        std::cout << "Expose::GetHeightMap(): --> Setting data_reader waitcondition" << std::endl;
        // Creating the infrastructure that allows an application thread to block
        // until some condition becomes true, such as data availability.
        // Create a Status Condition for the reader
        // Create DataReaderListeners 
        std::promise<std::string> theHeightMapId;
        std::future<std::string> f = theHeightMapId.get_future();
        DataReaderListenerImpl* listener_impl = new DataReaderListenerImpl(myRepo, &theHeightMapId); // will need to be deleted
        DDS::DataReaderListener_var waferheightmap_listener(listener_impl);
        waferheightmap_dr->ptr()->set_listener(waferheightmap_listener, DDS::DATA_AVAILABLE_STATUS | DDS::LIVELINESS_CHANGED_STATUS);
        std::string ret = f.get(); // Wait for the future ;-)
        std::cout << "returned heightmap Id: " << ret << std::endl;
        return ret;
    }

    void Expose::exposeWafer(std::string waferID)
    {
        // expose the whole wafer die by die with the provided image
        // Uses the wafer heightmap for lens correction.
        // Two phases repeat: stepping phase (to the next die) and scanning phase (of one die)
        std::string foundHeightMapId = StartHeightMapListener();
        WaferHeightMap whm_clone = myRepo->Get(foundHeightMapId);
        if (waferID == whm_clone.GetWaferId())
        {
            std::cout << "SUCCESS: WAFER IDs MATCH BETWEEN EXPOSE AND LEVELING" << std::endl;
        }
        whm_clone.LogHeightMap(); // Prove that we got the heightmap in the expose repository
    }
}}