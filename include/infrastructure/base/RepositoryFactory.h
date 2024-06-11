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
	//std::shared_ptr<ravendb::client::documents::DocumentStore> doc_store;
	//std::shared_ptr<hiberlite::Database> hiberDb;
public:
	IRepositoryBase<T> *GetRepository(RepositoryType repository, std::any db = nullptr)
	{
		GSL::Dprintf(GSL::DEBUG, " RespositoryFactory GetRepository enter with db type = ", db.type().name());
		switch (repository)
		{
		case RepositoryType::HeapRepository:
			if (!heapRep)
			{
				heapRep = new RepositoryHeapMemoryBase<T>();
			}
			return heapRep;
			break;
		case RepositoryType::ORM:
			if (!ormRep){
				GSL::Dprintf(GSL::DEBUG, " RespositoryFactory creating hiberlite RepositoryORMBase");
				auto hiberDb = std::any_cast<std::shared_ptr<hiberlite::Database>>(db);
				ormRep =  new RepositoryORMBase<T>(hiberDb);
			}
			return ormRep;
			break;
		case RepositoryType::FFS:
			if (!ffsRep)
			{
				ffsRep =  new RepositoryFFSBase<T>();
			}
			return ffsRep;
			break;
		case RepositoryType::ODM:
			GSL::Dprintf(GSL::DEBUG, " RespositoryFactory creating ravendb RepositoryODMBase");
			if (!odmRep)
			{
				auto doc_store = std::any_cast<std::shared_ptr<ravendb::client::documents::DocumentStore>>(db);
				odmRep = new RepositoryODMBase<T>(doc_store);
			}
			return odmRep;
			break;
		default:
			return 0;
			break;
		}
	}
	virtual ~RepositoryFactory()
	{
		if (ormRep) {delete ormRep;	ormRep = nullptr;}
		if (odmRep) {delete odmRep;	odmRep = nullptr;}
		if (ffsRep) {delete ffsRep;	ffsRep = nullptr;}
		if (heapRep) {delete heapRep; heapRep = nullptr;}
	};
};
