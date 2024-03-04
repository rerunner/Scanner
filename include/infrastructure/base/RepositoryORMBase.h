#pragma once

#include <iostream>
#include <vector>
#include <iterator>
#include "IRepositoryBase.h"
#include "hiberlite.h"

template <typename RepositoryBaseType>
class RepositoryORMBase : public IRepositoryBase<RepositoryBaseType>
{
private:
  hiberlite::Database *db;
public:
  //Constructor creates DB
  RepositoryORMBase()
  {
    db = new hiberlite::Database("sample.db");
    db->registerBeanClass<RepositoryBaseType>();
    db->dropModel();
    db->createModel();
  }
  
  void Store(RepositoryBaseType entity)
  {
    hiberlite::bean_ptr<RepositoryBaseType> managedCopyPtr = db->copyBean(entity);
  };
	
  void Delete(RepositoryBaseType entity)
  {
    std::vector<hiberlite::bean_ptr<RepositoryBaseType>> v = db->getAllBeans<RepositoryBaseType>();

    //Iterate over vector to find the wanted Id
    std::string id = entity.GetId();
    int teller = 0;
    for (auto &iter:v)
    {
      teller = iter.get_id();
      hiberlite::bean_ptr<RepositoryBaseType> xptr = db->loadBean<RepositoryBaseType>(teller);
      if (id.compare((*xptr).GetId()) == 0)
      {
	xptr.destroy();
	break;
      }
    }
    return;
  };
	
  RepositoryBaseType Get(std::string id)
  {
    std::vector<hiberlite::bean_ptr<RepositoryBaseType>> v = db->getAllBeans<RepositoryBaseType>();

    for (auto &iter:v)
    {
      hiberlite::bean_ptr<RepositoryBaseType> xptr = db->loadBean<RepositoryBaseType>(iter.get_id());
      if (id.compare((*xptr).GetId()) == 0)
      {
	return (*xptr);
      }
    }
    throw std::runtime_error(std::string("Object not found in repository\n"));
  };
};
