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
    GSL::Dprintf(GSL::DEBUG, "ENTERING Store");
    std::scoped_lock lock{repMtx};
    GSL::Dprintf(GSL::DEBUG, "Store Got Mutex");
    hiberlite::bean_ptr<RepositoryBaseType> managedCopyPtr = db->copyBean(entity);
    GSL::Dprintf(GSL::DEBUG, "LEAVING Store");
  };
	
  void Delete(RepositoryBaseType entity)
  {
    GSL::Dprintf(GSL::DEBUG, "ENTERING Delete");
    std::scoped_lock lock{repMtx};
    GSL::Dprintf(GSL::DEBUG, "Delete Got Mutex");
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
        GSL::Dprintf(GSL::DEBUG, "Delete -----> Match found");
	      xptr.destroy();
	      break;
      }
    }
    GSL::Dprintf(GSL::DEBUG, "LEAVING Delete");
  };
	
  RepositoryBaseType Get(Uuid requestedId)
  {
    GSL::Dprintf(GSL::DEBUG, "ENTERING Get");
    std::scoped_lock lock{repMtx};
    GSL::Dprintf(GSL::DEBUG, "Get Got Mutex");
    std::vector<hiberlite::bean_ptr<RepositoryBaseType>> v = db->getAllBeans<RepositoryBaseType>();

    for (auto &iter:v)
    {
      hiberlite::bean_ptr<RepositoryBaseType> xptr = db->loadBean<RepositoryBaseType>(iter.get_id());
      if (requestedId.Get().compare((*xptr).GetId().Get()) == 0)
      {
        GSL::Dprintf(GSL::DEBUG, "LEAVING Get");
	      return (*xptr);
      }
    }
    GSL::Dprintf(GSL::ERROR, "Object ", requestedId.Get(), " not found in repository");
    throw std::runtime_error(std::string("Object not found in repository\n"));
  };

  std::vector<RepositoryBaseType> GetAll()
  {
    GSL::Dprintf(GSL::DEBUG, "ENTERING GetAll");
    std::scoped_lock lock{repMtx};
    GSL::Dprintf(GSL::DEBUG, "GetAll Got Mutex");
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
			GSL::Dprintf(GSL::DEBUG, "getAllBeans failed: ", e.what());
		}
    GSL::Dprintf(GSL::DEBUG, "LEAVING GetAll");
    return vList;
  };

  std::vector<RepositoryBaseType> GetAllChildren(Uuid parentId)
  {
    GSL::Dprintf(GSL::DEBUG, "ENTERING GetAllChildren");
    std::scoped_lock lock{repMtx};
    GSL::Dprintf(GSL::DEBUG, "GetAllChildren Got Mutex");
    std::vector<RepositoryBaseType> vList;
    try {
			std::vector<hiberlite::bean_ptr<RepositoryBaseType>> v = db->getAllBeans<RepositoryBaseType>();
      for (auto &iter:v)
      {
        hiberlite::bean_ptr<RepositoryBaseType> xptr = db->loadBean<RepositoryBaseType>(iter.get_id());
        if (parentId.Get().compare((*xptr).GetParentId().Get()) == 0) // TODO: make WaferId ParentId.
        {
          GSL::Dprintf(GSL::DEBUG, "GetAllChildren -----> Parent match");
          vList.push_back(*xptr);
        }
      }
    }
		catch (std::exception& e) {
			GSL::Dprintf(GSL::DEBUG, "getAllBeans failed: ", e.what());
		}
    GSL::Dprintf(GSL::DEBUG, "LEAVING GetAllChildren");
    return vList;
  };
};
