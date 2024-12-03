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

#include "domain/MarkMeasurement.hpp"
#include "GenLogger.hpp"

using namespace Verdi;

//DataReaderListenerImpl::DataReaderListenerImpl (const DataReaderListenerImpl &other) : whmContext(other.whmContext)
DataReaderListenerImpl::DataReaderListenerImpl (const DataReaderListenerImpl &other) : whmContextFactory(other.whmContextFactory)
{
  GSL::Dprintf(GSL::DEBUG, "DataReaderListenerImpl constructed from other");
}

//DataReaderListenerImpl::DataReaderListenerImpl(std::unique_ptr<UnitOfWork> & passedWhmContext, std::promise<std::string>* heightmapId) : whmContext(passedWhmContext)
DataReaderListenerImpl::DataReaderListenerImpl(UnitOfWorkFactory *passedWhmContextFactory) : whmContextFactory(passedWhmContextFactory)
{
  GSL::Dprintf(GSL::DEBUG, "DataReaderListenerImpl listening for dds data to get the heightmap");
}

DataReaderListenerImpl::~DataReaderListenerImpl()
{
  GSL::Dprintf(GSL::DEBUG, "DataReaderListenerImpl DELETED");
}

void DataReaderListenerImpl::on_data_available(DDS::DataReader_ptr reader)
{
  GSL::Dprintf(GSL::DEBUG, "ENTER");
  
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
      std::unique_ptr<UnitOfWork> whmContext = whmContextFactory->GetNewUnitOfWork();
      GSL::Dprintf(GSL::DEBUG, "[DDS] Expose received WaferID = ", whm.waferID , " going to copy it.");
      // Translate from received DTO to local representation
      std::ostringstream oss;
      oss << whm.waferID; // Is there a better way from TAO managed string to std::string?
      std::string waferUuidString = oss.str();
      Uuid waferUuid(waferUuidString);
      std::shared_ptr<LevelingContext::WaferHeightMap> myHeightMap = std::make_shared<LevelingContext::WaferHeightMap>(waferUuid);
      for (int i = 0; i < scanner::generated::MAX_MEASUREMENT_STEPS ; i++)
      {
        LevelingContext::Position myPosition(whm.measurements[i].xyPosition.xPos, whm.measurements[i].xyPosition.yPos);
        double myZpos = whm.measurements[i].zPos;
        LevelingContext::MarkMeasurement myMeas(myPosition, myZpos);
        myHeightMap->AddMarkMeasurement(myMeas);
      }
      whmContext->RegisterNew<LevelingContext::WaferHeightMap>(myHeightMap);
      whmContext->Commit();
    }
    else if (status == DDS::RETCODE_NO_DATA)
    {
      GSL::Dprintf(GSL::ERROR, "reader received DDS::RETCODE_NO_DATA!");
    }
    else
    {
      GSL::Dprintf(GSL::ERROR, "read WaferHeightMap got Error: ", status);
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
