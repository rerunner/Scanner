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

#include "Uuid.hpp"
#include "infrastructure/base/RepositoryFactory.h"
#include "infrastructure/IWaferHeightMapRepository.hpp"
#include "DataReaderListenerImpl.h"

namespace Expose { namespace Application
{
  class Expose
  {
    private:
    DDS::DomainId_t WAFER_DOMAIN_ID;
    // Objects to block a thread until chocolate lot state update arrives
	  DDS::WaitSet_ptr _waitSet;
	  DDS::StatusCondition_ptr _condition;
    DDS::DataReader_var *waferheightmap_dr;
    std::unique_ptr<DataReaderListenerImpl> listener_impl;

    void Subscribe();
    void UnSubscribe();
    std::string StartHeightMapListener();

    public:
    Expose();
    ~Expose();
    void dummyMethod(Uuid waferId){}
    void exposeWafer(Uuid waferId); // expose the measured wafer identified with waferID
  };
}}

#endif // EXPOSE_H