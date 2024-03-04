#pragma once

#if 0
#include "IRepositoryFactory.h"
#include "IMyEntityRepository.h"

class MyAggregateRootRepository : public IMyAggregateRootRepository
{
	//IRepositoryFactory<MyEntity> RepositoryEntityFactory;
public:
	virtual void Store(MyEntity entity) { RepositoryBase::Store(entity); };
	virtual void Delete(MyEntity entity) { RepositoryBase::Delete(entity); };
	MyEntity Get(int id) { return RepositoryBase::Get(id); };
};
#endif
