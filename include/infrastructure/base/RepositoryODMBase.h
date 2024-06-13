#pragma once

#include <iostream>
#include <vector>
#include <iterator>
#include <iostream>
#include <string>
#include <string_view>
#include <mutex>
#include <fstream>
#include <cstdio>
#include <nlohmann/json.hpp>
#include <raven_cpp_client.h> // Problems with clang, gcc ok
#include "IRepositoryBase.h"
#include "GenLogger.hpp"

using json = nlohmann::json;

template <typename RepositoryBaseType>
class RepositoryODMBase : public IRepositoryBase<RepositoryBaseType>
{
private:
  inline static std::mutex repMtx;
  std::shared_ptr<ravendb::client::documents::DocumentStore> doc_store;
public:
  RepositoryODMBase(std::shared_ptr<ravendb::client::documents::DocumentStore> passedDocStore)
  {
    doc_store = passedDocStore;
  }
  virtual ~RepositoryODMBase(){}
  
  void Store(RepositoryBaseType entity)
  {
    std::scoped_lock lock{repMtx};
    auto entity_record = std::make_shared<RepositoryBaseType>(entity);
    auto session = doc_store->open_session();
    
    session.store(entity_record, entity.GetId().Get());
    session.save_changes(); //Commits the record to the database
  }
	
  void Delete(RepositoryBaseType entity)
  {
    std::scoped_lock lock{repMtx};
    auto session = doc_store->open_session();   
    session.delete_document(entity.GetId().Get());
    session.save_changes(); //Commits the record to the database
  }
	
  RepositoryBaseType Get(Uuid requestedId)
  {
    std::scoped_lock lock{repMtx};
    auto session = doc_store->open_session();
    //Load a document by its id
    std::shared_ptr<RepositoryBaseType> entity = session.load<RepositoryBaseType>(requestedId.Get());
    return *entity;
  }

  std::vector<RepositoryBaseType> GetAll()
  {
    std::scoped_lock lock{repMtx};
    std::vector<RepositoryBaseType> vList;

    GSL::Dprintf(GSL::INFO, "GetAll() ENTER");

    // Setup the query
    auto session = doc_store->open_session();
    std::ostringstream aggregation_rql_query;
    //Find all documents containing a parentId_ with matching uuid string
		aggregation_rql_query << "from '@all_docs'";
    auto query = session.advanced().raw_query<RepositoryBaseType>(aggregation_rql_query.str())->to_list();
    session.save_changes();

    if (query.size() > 0)
    {
      for (auto &iter:query)
      {
        vList.push_back(*iter);
      }
    }
    {
      GSL::Dprintf(GSL::WARNING, "GetAll() found 0 items!");
    }
    return vList;
  }

  std::vector<RepositoryBaseType> GetAllChildren(Uuid parentId)
  {
    std::scoped_lock lock{repMtx};
    std::vector<RepositoryBaseType> vList;
    
    // Setup the query
    auto session = doc_store->open_session();
    std::ostringstream aggregation_rql_query;
    //Find all documents containing a parentId_ with matching uuid string
		aggregation_rql_query << "from '@all_docs' where parentId_.uuid_ == '" << parentId.Get() << "'";
    GSL::Dprintf(GSL::DEBUG, "GetAllChildren query will be ", aggregation_rql_query.str());
    auto query = session.advanced().raw_query<RepositoryBaseType>(aggregation_rql_query.str())->to_list();
    session.save_changes();

    if (query.size() > 0)
    {
      for (auto &iter:query)
      {
        vList.push_back(*iter);
      }
    }
    else
    {
      GSL::Dprintf(GSL::WARNING, "Query ", aggregation_rql_query.str(), " found ", query.size(), " items!");
    }
    return vList;
  }
};
