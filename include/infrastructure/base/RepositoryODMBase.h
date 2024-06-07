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
    session.store(entity_record);
    session.save_changes(); //Commits the record to the database
  }
	
  void Delete(RepositoryBaseType entity)
  {
    std::scoped_lock lock{repMtx};
    auto entity_record = std::make_shared<RepositoryBaseType>(entity);
    auto session = doc_store->open_session();
    session.delete_document(entity_record);
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

    // Setup the query
    auto session = doc_store->open_session();
    auto query = session.query<RepositoryBaseType>();

    // TODO 
    
    //for (auto &iter:allEntities)
    //{
    //  vList.push_back(*iter);
    //}

    return vList;
  }

  std::vector<RepositoryBaseType> GetAllChildren(Uuid parentId)
  {
    std::scoped_lock lock{repMtx};
    std::vector<RepositoryBaseType> vList;
    std::vector<RepositoryBaseType> finalList;

    vList = GetAll();

    for (auto &iter:vList)
    {
      if (parentId.Get().compare(iter.GetParentId().Get()) == 0)
      {
        finalList.push_back(iter);
      }
    }    

    return finalList;
  }
};
