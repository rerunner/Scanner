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
    IRepositoryFactory<WaferHeightMap> *repositoryFactory;
    IRepositoryBase<WaferHeightMap> *myRepo;

    // constants & declarations for DDS WaferHeightMap domain Id, types, and topic
    DDS::DomainId_t WAFER_DOMAIN_ID;
    DDS::Topic_var waferheightmap_topic;
    //scanner::generated::WaferHeightMapTypeSupport_var waferheightmap_servant;
    DDS::Publisher_var pub;
    scanner::generated::WaferHeightMap whm_evt;
    DDS::InstanceHandle_t whm_handle;
    DDS::DataWriter_var waferHeightMap_base_dw;
    scanner::generated::WaferHeightMapDataWriter_var waferHeightMap_dw;

    void SetupDataWriter(); 
    void Publish(std::string waferHeightMapId); 

    public:
    Leveling();

    void dummyMethod(std::string waferId){}

    std::string measureWafer(std::string waferId); // returns the measured waferheightmap ID of the wafer
  };
}}

#endif