#pragma once

#include "IRepositoryFactory.h"
#include "RepositoryHeapMemoryBase.h"
#include "RepositoryORMBase.h"

template <typename T>
class RepositoryFactory : public IRepositoryFactory<T>
{
public:
	IRepositoryBase<T> *GetRepository(RepositoryType repository)
	{
		switch (repository)
		{
		case RepositoryType::HeapRepository:
			return new RepositoryHeapMemoryBase<T>();
			break;
		case RepositoryType::ORM:
			return new RepositoryORMBase<T>();
			break;
		default:
			return 0;
			break;
		}
	}
};
