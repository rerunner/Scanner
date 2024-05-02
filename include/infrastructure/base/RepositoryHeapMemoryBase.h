#pragma once

#include <iostream>
#include <vector>
#include <iterator>
#include "IRepositoryBase.h"

template <typename RepositoryBaseType>
class RepositoryHeapMemoryBase : public IRepositoryBase<RepositoryBaseType>
{
private:
	std::vector<RepositoryBaseType> entityVector;
public:
  void Store(RepositoryBaseType entity)
  {
    entityVector.push_back(entity); 
  };
	
  void Delete(RepositoryBaseType entity)
  {
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
    for (auto &iter:entityVector)
    {
      if (iter.GetId().Get() == id.Get())
      {
        return iter;
      }
    }
    throw std::runtime_error(std::string("Entity not found in repository\n"));
  };

  std::vector<RepositoryBaseType> GetAll()
  {
    return entityVector;
  };
};
