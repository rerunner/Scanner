/*
 *
 *
 * Distributed under the OpenDDS License.
 * See: http://www.opendds.org/license.html
 */

#ifndef DATAREADER_LISTENER_IMPL_H
#define DATAREADER_LISTENER_IMPL_H

#include <future>

#include <ace/Global_Macros.h>

#include <dds/DdsDcpsSubscriptionC.h>
#include <dds/DCPS/LocalObject.h>
#include <dds/DCPS/Definitions.h>

#include "domain/Exposure.hpp"
#include "domain/WaferHeightMap.hpp"
#include "infrastructure/base/UnitOfWork.hpp"

using namespace Verdi::unitofwork;

class DataReaderListenerImpl : public virtual OpenDDS::DCPS::LocalObject<DDS::DataReaderListener>
{
private:
  CORBA::Boolean is_exchange_closed_received_;
  UnitOfWorkFactory *whmContextFactory;
public:
  DataReaderListenerImpl(const DataReaderListenerImpl &); // Avoid Corba object base class implicit delete copy constructor error
  DataReaderListenerImpl(UnitOfWorkFactory *passedContext);
  virtual ~DataReaderListenerImpl();
  // DDS calls on_data_available on the listener for each
  // received WaferHeightMap sample.
  virtual void on_data_available(DDS::DataReader_ptr reader);
  virtual CORBA::Boolean is_exchange_closed_received();

  virtual void on_requested_deadline_missed(DDS::DataReader_ptr reader, const DDS::RequestedDeadlineMissedStatus& status){}
  virtual void on_requested_incompatible_qos(DDS::DataReader_ptr reader, const DDS::RequestedIncompatibleQosStatus& status){}
  virtual void on_sample_rejected(DDS::DataReader_ptr reader, const DDS::SampleRejectedStatus& status){}
  virtual void on_liveliness_changed(DDS::DataReader_ptr reader, const DDS::LivelinessChangedStatus& status){}
  virtual void on_subscription_matched(DDS::DataReader_ptr reader, const DDS::SubscriptionMatchedStatus& status){}
  virtual void on_sample_lost(DDS::DataReader_ptr reader, const DDS::SampleLostStatus& status){}
};

#endif /* DATAREADER_LISTENER_IMPL_H */
