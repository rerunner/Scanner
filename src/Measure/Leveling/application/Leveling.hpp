#ifndef LEVELING_H
#define LEVELING_H

// Data Distribution Service
#include "ScannerTypeSupportImpl.h"
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/PublisherImpl.h>
#include "dds/DCPS/StaticIncludes.h"

#include "domain/WaferHeightMap.hpp"
#include "infrastructure/base/RepositoryFactory.h"
#include "infrastructure/IWaferHeightMapRepository.hpp"

namespace Leveling  { namespace Application
{
  class Leveling
  {
    private:
    // Repository
    std::unique_ptr<IRepositoryFactory<WaferHeightMap>> repositoryFactory;
    IRepositoryBase<WaferHeightMap> *myRepo;

    // constants & declarations for DDS WaferHeightMap domain Id, types, and topic
    std::unique_ptr<DDS::DomainParticipantFactory_var> dpf;
    std::unique_ptr<DDS::DomainParticipant_var> participant;
    DDS::DomainId_t WAFER_DOMAIN_ID;
    DDS::Topic_var waferheightmap_topic;
    //scanner::generated::WaferHeightMapTypeSupport_var waferheightmap_servant;
    DDS::Publisher_var pub;
    scanner::generated::WaferHeightMap whm_evt;
    DDS::InstanceHandle_t whm_handle;
    DDS::DataWriter_var waferHeightMap_base_dw;
    scanner::generated::WaferHeightMapDataWriter_var waferHeightMap_dw;

    // Methods
    void SetupDataWriter(); 
    DDS::ReturnCode_t Publish(Uuid waferHeightMapId); 

    public:
    Leveling();
    virtual ~Leveling(){}

    void dummyMethod(Uuid waferId){}

    Uuid measureWafer(Uuid waferId); // returns the measured waferheightmap ID of the wafer
  };
}}

#endif