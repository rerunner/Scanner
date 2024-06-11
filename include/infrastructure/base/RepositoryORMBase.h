#pragma once

#include <iostream>
#include <vector>
#include <iterator>
#include <iostream>
#include <string>
#include <string_view>
#include <mutex>
#include "hiberlite.h"
#include "IRepositoryBase.h"
#include "GenLogger.hpp"

template <typename RepositoryBaseType>
class RepositoryORMBase : public IRepositoryBase<RepositoryBaseType>
{
private:
  //hiberlite::Database *db;
  std::shared_ptr<hiberlite::Database> db;
  inline static std::mutex repMtx;
public:
  //Constructor receives DB
  RepositoryORMBase(std::shared_ptr<hiberlite::Database> passedDb)
  {
    db = passedDb;
  }

  virtual ~RepositoryORMBase()
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
    try {
      std::vector<hiberlite::bean_ptr<RepositoryBaseType>> v = db->getAllBeans<RepositoryBaseType>();
      for (auto &iter:v)
      {
        hiberlite::bean_ptr<RepositoryBaseType> xptr = db->loadBean<RepositoryBaseType>(iter.get_id());
        vList.push_back(*xptr);
      }
    }
    catch (std::exception& e) {
			GSL::Dprintf(GSL::ERROR, "getAllBeans failed: ", e.what());
		}
    return vList;
  };

  std::vector<RepositoryBaseType> GetAllChildren(Uuid parentId)
  {
    std::scoped_lock lock{repMtx};
    std::vector<RepositoryBaseType> vList;
    try {
      std::ostringstream oss;
      oss << "parentId__uuid_" << " = '" << parentId.Get() << "'"; // SQL syntax, column = "parentId__uuid_" and value is the uuid string
      std::string sqlQuery = oss.str();
      std::vector<hiberlite::bean_ptr<RepositoryBaseType>> v = db->getBeansByQuery<RepositoryBaseType>(sqlQuery);
      for (auto &iter:v)
      {
        hiberlite::bean_ptr<RepositoryBaseType> xptr = db->loadBean<RepositoryBaseType>(iter.get_id());
        vList.push_back(*xptr);
      }
    }
		catch (std::exception& e) {
			GSL::Dprintf(GSL::ERROR, "getAllBeans failed: ", e.what());
		}
    return vList;
  };
};
