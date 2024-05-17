#pragma once

#include "IRepositoryFactory.h"
#include "RepositoryHeapMemoryBase.h"
#include "RepositoryORMBase.h"

template <typename T>
class RepositoryFactory : public IRepositoryFactory<T>
{
private:
	RepositoryHeapMemoryBase<T> *heapRep;
	RepositoryORMBase<T> *ormRep;

public:
	IRepositoryBase<T> *GetRepository(RepositoryType repository, hiberlite::Database *db = nullptr)
	{
		switch (repository)
		{
		case RepositoryType::HeapRepository:
			heapRep = new RepositoryHeapMemoryBase<T>();
			return heapRep;
			break;
		case RepositoryType::ORM:
			ormRep =  new RepositoryORMBase<T>(db);
			return ormRep;
			break;
		default:
			return 0;
			break;
		}
	}
	virtual ~RepositoryFactory()
	{
		if (ormRep) {delete ormRep;	}
		if (heapRep) {delete heapRep;}
	};
};
