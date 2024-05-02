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

template <typename RepositoryBaseType>
class RepositoryORMBase : public IRepositoryBase<RepositoryBaseType>
{
private:
  hiberlite::Database db;
public:
  //Constructor creates DB
  RepositoryORMBase()
  {
    std::string totalProcessName = program_invocation_name; // Linux specific
    std::size_t processNamePos = totalProcessName.find_last_of("/\\");
    std::string justProcessName = totalProcessName.substr(processNamePos+1);
    std::ostringstream databaseName;
    databaseName << justProcessName << "Database.db";
    GSL::Dprintf(GSL::DEBUG, "Opening ", databaseName.str());
    db.open(databaseName.str());
    db.registerBeanClass<RepositoryBaseType>();
    
    //db.dropModel(); --> Probably some recovery mode can call this
    try {
      db.createModel();
    }
    catch (std::exception& e) {
      GSL::Dprintf(GSL::WARNING, "didn't create the tables: ", e.what());
    }
  }
  
  void Store(RepositoryBaseType entity)
  {
    hiberlite::bean_ptr<RepositoryBaseType> managedCopyPtr = db.copyBean(entity);
  };
	
  void Delete(RepositoryBaseType entity)
  {
    std::vector<hiberlite::bean_ptr<RepositoryBaseType>> v = db.getAllBeans<RepositoryBaseType>();

    //Iterate over vector to find the wanted Id
    std::string id = entity.GetId().Get();
    int teller = 0;
    for (auto &iter:v)
    {
      teller = iter.get_id();
      hiberlite::bean_ptr<RepositoryBaseType> xptr = db.loadBean<RepositoryBaseType>(teller);
      if (id.compare((*xptr).GetId().Get()) == 0)
      {
	      xptr.destroy();
	      break;
      }
    }
    return;
  };
	
  RepositoryBaseType Get(Uuid requestedId)
  {
    std::vector<hiberlite::bean_ptr<RepositoryBaseType>> v = db.getAllBeans<RepositoryBaseType>();

    for (auto &iter:v)
    {
      hiberlite::bean_ptr<RepositoryBaseType> xptr = db.loadBean<RepositoryBaseType>(iter.get_id());
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
    std::vector<RepositoryBaseType> vList;
    GSL::Dprintf(GSL::INFO, "GetAll ENTER");
    std::vector<hiberlite::bean_ptr<RepositoryBaseType>> v = db.getAllBeans<RepositoryBaseType>();
    for (auto &iter:v)
    {
      GSL::Dprintf(GSL::INFO, "GetAll FOUND ONE ENTRY");
      hiberlite::bean_ptr<RepositoryBaseType> xptr = db.loadBean<RepositoryBaseType>(iter.get_id());
      vList.push_back(*xptr);
    }
    GSL::Dprintf(GSL::INFO, "GetAll LEAVING");
    return vList;
  };
};
