#pragma once

#include "IRepositoryBase.h"

enum RepositoryType
{
  HeapRepository,
  ORM
};

template <typename T>
class IRepositoryFactory
{
public:
	virtual IRepositoryBase<T>* GetRepository(RepositoryType repository) = 0;
};
