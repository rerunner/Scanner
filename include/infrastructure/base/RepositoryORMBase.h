#pragma once

#include "GenLogger.hpp"
#include <iostream>
#include <vector>
#include <iterator>
#include "IRepositoryBase.h"
#include "hiberlite.h"
#include <iostream>
#include <string>
#include <string_view>
#include <mutex>

template <typename RepositoryBaseType>
class RepositoryORMBase : public IRepositoryBase<RepositoryBaseType>
{
private:
  hiberlite::Database *db;
  inline static std::mutex repMtx;
public:
  //Constructor creates DB
  RepositoryORMBase(hiberlite::Database *passedDb)
  {
    db = passedDb;
  }

  ~RepositoryORMBase()
  {
    if (db)
    {
      db = nullptr;
    }
  }
  
  void Store(RepositoryBaseType entity)
  {
    std::scoped_lock lock{repMtx};
    hiberlite::bean_ptr<RepositoryBaseType> managedCopyPtr = db->copyBean(entity);
  };
	
  void Delete(RepositoryBaseType entity)
  {
    std::scoped_lock lock{repMtx};
    std::vector<hiberlite::bean_ptr<RepositoryBaseType>> v = db->getAllBeans<RepositoryBaseType>();

    //Iterate over vector to find the wanted Id
    std::string id = entity.GetId().Get();
    int teller = 0;
    for (auto &iter:v)
    {
      teller = iter.get_id();
      hiberlite::bean_ptr<RepositoryBaseType> xptr = db->loadBean<RepositoryBaseType>(teller);
      if (id.compare((*xptr).GetId().Get()) == 0)
      {
	      xptr.destroy();
	      break;
      }
    }
  };
	
  RepositoryBaseType Get(Uuid requestedId)
  {
    std::scoped_lock lock{repMtx};
    std::vector<hiberlite::bean_ptr<RepositoryBaseType>> v = db->getAllBeans<RepositoryBaseType>();

    for (auto &iter:v)
    {
      hiberlite::bean_ptr<RepositoryBaseType> xptr = db->loadBean<RepositoryBaseType>(iter.get_id());
      if (requestedId.Get().compare((*xptr).GetId().Get()) == 0)
      {
	      return (*xptr);
      }
    }
    GSL::Dprintf(GSL::ERROR, "Object ", requestedId.Get(), " not found in repository");
    throw std::runtime_error(std::string("Object not found in repository\n"));
  };

  std::vector<RepositoryBaseType> GetAll()
  {
    std::scoped_lock lock{repMtx};
    std::vector<RepositoryBaseType> vList;
    std::vector<hiberlite::bean_ptr<RepositoryBaseType>> v = db->getAllBeans<RepositoryBaseType>();
    for (auto &iter:v)
    {
      hiberlite::bean_ptr<RepositoryBaseType> xptr = db->loadBean<RepositoryBaseType>(iter.get_id());
      vList.push_back(*xptr);
    }
    return vList;
  };

  std::vector<RepositoryBaseType> GetAllChildren(Uuid parentId)
  {
    std::scoped_lock lock{repMtx};
    std::vector<RepositoryBaseType> vList;
    std::vector<hiberlite::bean_ptr<RepositoryBaseType>> v = db->getAllBeans<RepositoryBaseType>();
    for (auto &iter:v)
    {
      hiberlite::bean_ptr<RepositoryBaseType> xptr = db->loadBean<RepositoryBaseType>(iter.get_id());
      if (parentId.Get().compare((*xptr).GetParentId().Get()) == 0) // TODO: make WaferId ParentId.
      {
        vList.push_back(*xptr);
      }
    }
    return vList;
  };
};
