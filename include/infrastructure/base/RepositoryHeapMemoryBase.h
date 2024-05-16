#pragma once

#include <iostream>
#include <vector>
#include <iterator>
#include "IRepositoryBase.h"
#include "GenLogger.hpp"

template <typename RepositoryBaseType>
class RepositoryHeapMemoryBase : public IRepositoryBase<RepositoryBaseType>
{
private:
	inline static std::vector<RepositoryBaseType> entityVector;
  inline static std::mutex repMtx;
public:
  void Store(RepositoryBaseType entity)
  {
    std::scoped_lock lock{repMtx};
    entityVector.push_back(entity); 
  };
	
  void Delete(RepositoryBaseType entity)
  {
    std::scoped_lock lock{repMtx};
    for (auto i = entityVector.begin(); i != entityVector.end(); ++i)
    {
      RepositoryBaseType dummy = *i;
      if (dummy.GetId().Get() == entity.GetId().Get())
      {
        entityVector.erase(i);
        return;
      }
    }
    return;
  };
	
  RepositoryBaseType Get(Uuid id)
  {
    std::scoped_lock lock{repMtx};
    for (auto &iter:entityVector)
    {
      GSL::Dprintf(GSL::INFO, "Get -----> searching, found id = ", iter.GetId().Get());
      if (iter.GetId().Get() == id.Get())
      {
        GSL::Dprintf(GSL::INFO, "Get -----> Id match");
        return iter;
      }
    }
    throw std::runtime_error(std::string("Entity not found in repository\n"));
  };

  std::vector<RepositoryBaseType> GetAll()
  {
    std::scoped_lock lock{repMtx};
    return entityVector;
  };

  std::vector<RepositoryBaseType> GetAllChildren(Uuid parentId)
  {
    std::scoped_lock lock{repMtx};
    std::vector<RepositoryBaseType> vList;

    for (auto &iter:entityVector)
    {
      GSL::Dprintf(GSL::INFO, "GetAllChildren -----> searching, found parent id = ", iter.GetParentId().Get());
      if (parentId.Get().compare(iter.GetParentId().Get()) == 0)
      {
        GSL::Dprintf(GSL::INFO, "GetAllChildren -----> Parent match");
        vList.push_back(iter);
      }
    }
    return vList;
  };
};
