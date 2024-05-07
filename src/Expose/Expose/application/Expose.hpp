#ifndef EXPOSE_H
#define EXPOSE_H

// Data Distribution Service
#include "ScannerTypeSupportImpl.h"
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/PublisherImpl.h>
#include "dds/DCPS/StaticIncludes.h"

#include "dds/DCPS/Service_Participant.h"
#include "dds/DCPS/Marked_Default_Qos.h"
#include "dds/DCPS/SubscriberImpl.h"
// ?? #include "dds/DCPS/BuiltinTopicUtils.h"
#include "ace/streams.h"
#include "orbsvcs/Time_Utilities.h"

#include <dds/DCPS/WaitSet.h>

#include <cppkafka/cppkafka.h>

#include "Uuid.hpp"
#include "infrastructure/base/RepositoryFactory.h"
#include "infrastructure/IWaferHeightMapRepository.hpp"
#include "DataReaderListenerImpl.h"
#include "infrastructure/base/UnitOfWork.hpp"

using namespace unitofwork;

namespace Expose { namespace Application
{
  class Expose
  {
    private:
    UnitOfWorkFactory UoWFactory;
    // Kafka part
    std::unique_ptr<cppkafka::Configuration> kafkaConfig;
    std::unique_ptr<cppkafka::Consumer> kafkaConsumer;
    std::thread eventListenerThread;
    bool quit_;
    void eventListenerThreadHandler();
    // DDS part
    DDS::DomainId_t WAFER_DOMAIN_ID;
    // Objects to block a thread until chocolate lot state update arrives
	  DDS::WaitSet_ptr _waitSet;
	  DDS::StatusCondition_ptr _condition;
    DDS::DataReader_var *waferheightmap_dr;
    void Subscribe();
    void UnSubscribe();
    std::string StartHeightMapListener(std::unique_ptr<UnitOfWork> & context_);

    public:
    Expose();
    ~Expose();
    void dummyMethod(Uuid waferId){}
    void exposeWafer(Uuid waferId); // expose the measured wafer identified with waferID
  };
}}

#endif // EXPOSE_H