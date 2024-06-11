#pragma once

#include <any>
#include <raven_cpp_client.h> // Problems with clang, gcc ok
#include "IRepositoryBase.h"

enum RepositoryType
{
  HeapRepository,
  ORM,
  ODM,
  FFS
};

template <typename T>
class IRepositoryFactory
{
public:
  virtual ~IRepositoryFactory(){}
  virtual IRepositoryBase<T>* GetRepository(RepositoryType repository, std::any db) = 0;
};
