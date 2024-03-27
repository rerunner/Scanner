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

void DataReaderListenerImpl::on_data_available(DDS::DataReader_ptr reader)
{
  std::cout << "DATA FOUND!!!!" << std::endl;
  
  try
  {
    scanner::generated::WaferHeightMapDataReader_var heightmap_dr = scanner::generated::WaferHeightMapDataReader::_narrow(reader);

    if (!heightmap_dr)
    {
      std::cerr << "DataReaderListenerImpl:: " << "on_data_available:" << " _narrow failed." << std::endl;
      ACE_OS::exit(1);
    }
    scanner::generated::WaferHeightMap whm;
    DDS::SampleInfo si;

    DDS::ReturnCode_t status = heightmap_dr->take_next_sample(whm, si) ;

    if (status == DDS::RETCODE_OK) 
    {
      std::cout << "Expose: received WaferID = " << whm.waferID << std::endl;
      std::cout << "SampleInfo.sample_rank = " << si.sample_rank << std::endl;
      // Translate from received DTO to local representation
      for (int i = 0; i < 10000 ; i++)
      {
        Position myPosition(whm.measurements[i].xyPosition.xPos, whm.measurements[i].xyPosition.yPos);
        double myZpos = whm.measurements[i].zPos;
        Measurement myMeas(myPosition, myZpos);
        myHeightMap.AddMeasurement(myMeas);
      }
      myRepo->Store(myHeightMap);
      myHeightmapId->set_value(myHeightMap.GetId()); // signal the future ;-)
    }
    else if (status == DDS::RETCODE_NO_DATA)
    {
      std::cerr << "ERROR: reader received DDS::RETCODE_NO_DATA!" << std::endl;
    }
    else
    {
      std::cerr << "ERROR: read Quote: Error: " <<  status << std::endl;
    }
  } catch (CORBA::Exception& e) 
  {
    std::cerr << "Exception caught in read:" << std::endl << e << std::endl;
    ACE_OS::exit(1);
  }
}

CORBA::Boolean DataReaderListenerImpl::is_exchange_closed_received()
{
  //ACE_Guard<ACE_Mutex> guard(this->lock_);
  return this->is_exchange_closed_received_;
}
