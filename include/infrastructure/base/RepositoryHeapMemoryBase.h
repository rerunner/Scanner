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
    entityVector.push_back(entity); std::cout << "Entity " << entity.GetId() << " stored in repository!\n";
  };
	
  void Delete(RepositoryBaseType entity)
  {
    for (auto i = entityVector.begin(); i != entityVector.end(); ++i)
    {
      RepositoryBaseType dummy = *i;
      if (dummy.GetId() == entity.GetId())
      {
        std::cout << "Entity Id " << entity.GetId() << " erased from repository!\n";
        entityVector.erase(i);
        return;
      }
    }
    std::cout << "Entity Id " << entity.GetId() << " not found in repository!\n";
    return;
  };
	
  RepositoryBaseType Get(std::string id)
  {
    //for (const auto &iter:entityVector)
    for (auto &iter:entityVector)
    {
      if (iter.GetId() == id)
      {
        std::cout << "Entity " << iter.GetId() << " retrieved from repository!\n";
        return iter;
      }
    }
    std::cout << "Entity Id " << id << " not found in repository!\n";
    throw std::runtime_error(std::string("Entity not found in repository\n"));
  };
};
