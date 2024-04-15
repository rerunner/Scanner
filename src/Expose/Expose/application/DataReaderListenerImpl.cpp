/*
 *
 *
 * Distributed under the OpenDDS License.
 * See: http://www.opendds.org/license.html
 */

#include <ace/Log_Msg.h>
#include <ace/OS_NS_stdlib.h>

#include "DataReaderListenerImpl.h"
#include "ScannerTypeSupportC.h"
#include "ScannerTypeSupportImpl.h"

#include <iostream>

#include "domain/Measurement.hpp"
#include "GenLogger.hpp"


DataReaderListenerImpl::DataReaderListenerImpl()
{
  whmContext = NULL;
}

DataReaderListenerImpl::DataReaderListenerImpl (const DataReaderListenerImpl &other)
{
  whmContext = other.whmContext;
  myHeightmapId = other.myHeightmapId;
}

DataReaderListenerImpl::DataReaderListenerImpl(UnitOfWork<WaferHeightMap> *passedWhmContext, std::promise<std::string>* heightmapId)
{
  whmContext = passedWhmContext;
  myHeightmapId = heightmapId;
  GSL::Dprintf(GSL::INFO, "DataReaderListenerImpl listening for dds data to get the heightmap");
}

DataReaderListenerImpl::~DataReaderListenerImpl()
{
}

void DataReaderListenerImpl::on_data_available(DDS::DataReader_ptr reader)
{
  GSL::Dprintf(GSL::INFO, "DATA FOUND!");
  
  try
  {
    scanner::generated::WaferHeightMapDataReader_var heightmap_dr = scanner::generated::WaferHeightMapDataReader::_narrow(reader);

    if (!heightmap_dr)
    {
      GSL::Dprintf(GSL::ERROR, "_narrow failed.");
      ACE_OS::exit(1);
    }
    scanner::generated::WaferHeightMap whm;
    DDS::SampleInfo si;

    DDS::ReturnCode_t status = heightmap_dr->take_next_sample(whm, si) ;

    if (status == DDS::RETCODE_OK) 
    {
      GSL::Dprintf(GSL::INFO, "Expose: received WaferID = ", whm.waferID );
      GSL::Dprintf(GSL::INFO, "SampleInfo.sample_rank = ", si.sample_rank);
      // Translate from received DTO to local representation
      std::ostringstream oss;
      oss << whm.waferID; // Is there a better way from TAO managed string to std::string?
      WaferHeightMap myHeightMap(oss.str());
      for (int i = 0; i < 10000 ; i++)
      {
        Position myPosition(whm.measurements[i].xyPosition.xPos, whm.measurements[i].xyPosition.yPos);
        double myZpos = whm.measurements[i].zPos;
        Measurement myMeas(myPosition, myZpos);
        myHeightMap.AddMeasurement(myMeas);
      }
      whmContext->RegisterNew(myHeightMap);
      myHeightmapId->set_value(myHeightMap.GetId().Get()); // signal the future ;-)
    }
    else if (status == DDS::RETCODE_NO_DATA)
    {
      GSL::Dprintf(GSL::ERROR, "reader received DDS::RETCODE_NO_DATA!");
    }
    else
    {
      GSL::Dprintf(GSL::ERROR, "read Quote: Error: ", status);
    }
  } catch (CORBA::Exception& e) 
  {
    GSL::Dprintf(GSL::ERROR, "Exception caught in read: ", e);
    ACE_OS::exit(1);
  }
}

CORBA::Boolean DataReaderListenerImpl::is_exchange_closed_received()
{
  return this->is_exchange_closed_received_;
}
