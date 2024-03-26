#include "application/Expose.hpp"
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
        this->Subscribe();
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

        // Create DataReaderListeners 
        DataReaderListenerImpl* listener_impl = new DataReaderListenerImpl;
        DDS::DataReaderListener_var waferheightmap_listener(listener_impl);

        // Create the WaferHeightMap DataReader
        DDS::DataReaderQos dr_default_qos;
        sub->get_default_datareader_qos (dr_default_qos);

        DDS::DataReader_var waferheightmap_dr = sub->create_datareader(  waferheightmap_topic,
                                                                DATAREADER_QOS_DEFAULT,
                                                                waferheightmap_listener,
                                                                OpenDDS::DCPS::DEFAULT_STATUS_MASK);

        // This WaitSet object will be used to block a thread until one or more 
        // conditions become true.  In this case, there is a single condition that
        // will wake up the WaitSet when the reader receives data
        _waitSet = new DDS::WaitSet();
        if (_waitSet == NULL) 
        {
            std::stringstream errss;
            errss << "ChocolateLotStateReader(): failure to create WaitSet.";
            throw errss.str();
        }

        // Creating the infrastructure that allows an application thread to block
        // until some condition becomes true, such as data availability.
        _condition = waferheightmap_dr->get_statuscondition();

        // Wake up the thread when data is available
        _condition->set_enabled_statuses(DDS::DATA_AVAILABLE_STATUS); 
        if (_condition == NULL) 
        {
            std::stringstream errss;
            errss << "ChocolateLotStateReader(): failure to initialize condition.";
            throw errss.str();
        }

        // Attaching the condition to the WaitSet
        _waitSet->attach_condition(_condition);

        // Block thread for chocolate lot state updates to arrive
        DDS::ConditionSeq activeConditions;
	    // How long to block for data at a time
	    DDS::Duration_t timeout = { 3*60,0 }; // 3 minutes
	    DDS::ReturnCode_t retcode = _waitSet->wait(activeConditions, timeout);
        TheServiceParticipant->shutdown ();
    }

    void Expose::exposeWafer(std::string waferID)
    {
        // expose the whole wafer die by die with the provided image
        // Uses the wafer heightmap for lens correction.
        // Two phases repeat: stepping phase (to the next die) and scanning phase (of one die)
    }
}}