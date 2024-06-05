#pragma once

#include "IRepositoryFactory.h"
#include "RepositoryHeapMemoryBase.h"
#include "RepositoryORMBase.h"
#include "RepositoryODMBase.h"
#include "RepositoryFFSBase.h"

template <typename T>
class RepositoryFactory : public IRepositoryFactory<T>
{
private:
	RepositoryHeapMemoryBase<T> *heapRep;
	RepositoryORMBase<T> *ormRep;
	RepositoryODMBase<T> *odmRep;
	RepositoryFFSBase<T> *ffsRep;

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
		case RepositoryType::ODM:
			odmRep =  new RepositoryODMBase<T>();
			return odmRep;
			break;
		case RepositoryType::FFS:
			ffsRep =  new RepositoryFFSBase<T>();
			return ffsRep;
			break;
		default:
			return 0;
			break;
		}
	}
	virtual ~RepositoryFactory()
	{
		if (ormRep) {delete ormRep;	}
		if (odmRep) {delete odmRep;	}
		if (ffsRep) {delete ffsRep;	}
		if (heapRep) {delete heapRep;}
	};
};
