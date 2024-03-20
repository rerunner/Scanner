#ifndef LEVELING_H
#define LEVELING_H

// Data Distribution Service
#include "ScannerTypeSupportImpl.h"
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/PublisherImpl.h>
#include "dds/DCPS/StaticIncludes.h"

namespace Leveling
{
  class Leveling
  {
    private:
    // constants for Stock Quoter domain Id, types, and topic
    DDS::DomainId_t WAFER_DOMAIN_ID;
    const char* WAFER_HEIGHTMAP_TYPE;
    const char* WAFER_HEIGHTMAP_TOPIC;

    void SetupDataWriter(); 
    void Publish(); 

    public:
    Leveling(): WAFER_DOMAIN_ID(1066), WAFER_HEIGHTMAP_TYPE("Quote Type"), WAFER_HEIGHTMAP_TOPIC("Stock Quotes"){}

    void dummyMethod(std::string waferId){}

    void measureWafer(std::string waferId);
  };
}

#endif