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
	std::shared_ptr<ravendb::client::documents::DocumentStore> doc_store;
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
			doc_store = ravendb::client::documents::DocumentStore::create();
			doc_store->set_urls({ "http://127.0.0.1:8080" }); // port 8080
			doc_store->set_database("Scanner");
			doc_store->initialize();
			odmRep =  new RepositoryODMBase<T>(doc_store);
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
