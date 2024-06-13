#pragma once

#include <string>
#include <list>
#include <map>
#include <functional>
#include <cstdint>
#include <string>
#include <variant>
#include <typeinfo>
#include <type_traits>
#include <memory>
#include <unordered_set>
#include <any>
#include "Uuid.hpp"
#include "IRepositoryFactory.h"
#include "IRepositoryBase.h"
#include "RepositoryFactory.h" // ?
#include "RepositoryORMBase.h"
#include "RepositoryODMBase.h"
#include "RepositoryFFSBase.h"
#include "GenLogger.hpp"
#include "domain/WaferHeightMap.hpp"

//#define REPOSITORY_TYPE RepositoryType::HeapRepository
#define REPOSITORY_TYPE RepositoryType::ORM
//#define REPOSITORY_TYPE RepositoryType::FFS
//#define REPOSITORY_TYPE RepositoryType::ODM

namespace unitofwork {

typedef std::map<std::string, void*> Dict; // Dictionary for repositories

enum RegistryTypeEnum
{
    RegisterNew,
    RegisterDirty,
    RegisterClean,
    RegisterDeleted
};

template < typename EntityType> 
class EntityRegister
{
    std::shared_ptr<EntityType> entityInstance;
    RegistryTypeEnum registryType;
    //hiberlite::Database *db;
    std::any db;
    inline static std::mutex mtx;
    RepositoryType repositoryType_;

    void Commit()
    {
        std::scoped_lock lock{mtx};
        std::unique_ptr<IRepositoryFactory<EntityType>> repositoryFactory = std::make_unique<RepositoryFactory<EntityType>>();
        auto repository = repositoryFactory->GetRepository(repositoryType_, db);
        switch (registryType)
        {
            case RegistryTypeEnum::RegisterNew:
            case RegistryTypeEnum::RegisterDirty:
            case RegistryTypeEnum::RegisterClean:
                repository->Store(*entityInstance);
                break;
            case RegistryTypeEnum::RegisterDeleted:
                repository->Delete(*entityInstance);
                break;
            default:
                GSL::Dprintf(GSL::ERROR, "Commiter has no known RegistryType");
                break;
        }  
    }
public:
    EntityRegister(std::shared_ptr<EntityType> newEnt, RegistryTypeEnum newRegistryType, std::any passedDb)
    {
        entityInstance = newEnt;
        registryType = newRegistryType;
        db = passedDb;
        repositoryType_ = REPOSITORY_TYPE;
    }
    
    virtual ~EntityRegister()
    {
        Commit();
        db = nullptr;
    }
};

template<typename EntityType>
using EntityRegisterPtr = std::shared_ptr<EntityRegister<EntityType>>;

class UnitOfWork
{
private:
    Uuid _context;
    Dict repositories_;
    std::list<std::any> _newEntities;
    std::list<std::any> _updatedEntities;
    std::list<std::any> _deletedEntities;
    std::any db;
    RepositoryType repositoryType_;

public:
    UnitOfWork(std::any passedDb)
    {
        db = passedDb;
        repositoryType_ = REPOSITORY_TYPE;
    }
    virtual ~UnitOfWork()
    {
        db = nullptr;
    }

    template <typename EntityType>
    void RegisterNew(std::shared_ptr<EntityType> entPtr)
    {
        if (repositoryType_ == RepositoryType::ORM)
        {
            auto db2 = std::any_cast<std::shared_ptr<hiberlite::Database>>(db);
            try {
                db2->registerBeanClass<EntityType>();
            }
            catch (std::exception& e) {
                GSL::Dprintf(GSL::DEBUG, "didn't register beanclass: ", e.what());
            }
        }

        EntityRegisterPtr<EntityType> myNewEntityPtr = std::make_shared<EntityRegister<EntityType>>(entPtr, RegistryTypeEnum::RegisterNew, db);
        _newEntities.push_back(std::move(myNewEntityPtr)); //Register
        
    }

    template <typename EntityType>
    void RegisterDirty(std::shared_ptr<EntityType> entPtr)
    { 
        if (repositoryType_ == RepositoryType::ORM)
        {
            auto db2 = std::any_cast<std::shared_ptr<hiberlite::Database>>(db);
            try {
                db2->registerBeanClass<EntityType>();
            }
            catch (std::exception& e) {
                GSL::Dprintf(GSL::DEBUG, "didn't register beanclass: ", e.what());
            }
        }
        EntityRegisterPtr<EntityType> myUpdatedEntityPtr = std::make_shared<EntityRegister<EntityType>>(entPtr, RegistryTypeEnum::RegisterDirty, db);
        _updatedEntities.push_back(std::move(myUpdatedEntityPtr)); //Register
        
        GSL::Dprintf(GSL::DEBUG, "EXIT");
    }

    template <typename EntityType>
    void RegisterDeleted(std::shared_ptr<EntityType> entPtr)
    { 
        if (repositoryType_ == RepositoryType::ORM)
        {
            auto db2 = std::any_cast<std::shared_ptr<hiberlite::Database>>(db);
            try {
                db2->registerBeanClass<EntityType>();
            }
            catch (std::exception& e) {
                GSL::Dprintf(GSL::DEBUG, "didn't register beanclass: ", e.what());
            }
        }
		
        EntityRegisterPtr<EntityType> myDeletedEntityPtr = std::make_shared<EntityRegister<EntityType>>(entPtr, RegistryTypeEnum::RegisterDeleted, db);
        _deletedEntities.push_back(std::move(myDeletedEntityPtr)); //Register
    }
    
    void Commit()
    {
        GSL::Dprintf(GSL::DEBUG, "Commit UoW ID = ", _context.Get());
        // Too bad that we don't know the templated types anymore :-(
        // Committing changed or new objects happens in the destructor of list of changed entities (at destruction of this UoW instance)
        if (repositoryType_ == RepositoryType::ORM)
        {
            auto db2 = std::any_cast<std::shared_ptr<hiberlite::Database>>(db);
            try {
                db2->createModel();
            }
            catch (std::exception& e) {
                GSL::Dprintf(GSL::DEBUG, "didn't create the tables: ", e.what());
            }
        }
        _newEntities.clear(); // clearing calls destructor, hence commit happens
    }

    void Rollback()
    {
        //Todo, remove without commit
    }

    template <typename entityType>
    entityType Get(Uuid id){/*repository->Get(id);*/ return nullptr;}

    template <typename entityType>
    std::list<entityType> GetAll(){/*todo*/}
};


class UnitOfWorkFactory
{
private:
    std::any db;
    RepositoryType repositoryType_;
    std::any doc_store;
    std::any hiberDb;

    void OpenRavenDB()
    {
        doc_store = ravendb::client::documents::DocumentStore::create();
        auto doc_store2 = std::any_cast<std::shared_ptr<ravendb::client::documents::DocumentStore>>(doc_store);
        doc_store2->set_urls({ "http://127.0.0.1:8080" }); // port 8080

        std::string totalProcessName = program_invocation_name; // Linux specific
		std::size_t processNamePos = totalProcessName.find_last_of("/\\");
		std::string justProcessName = totalProcessName.substr(processNamePos+1);
		std::ostringstream databaseName;
		databaseName << justProcessName << "DB";
        doc_store2->set_database(databaseName.str());
        //doc_store2->set_database("Scanner");
        doc_store2->initialize();
    }

    void OpenHiberlite()
	{
		hiberDb = std::make_shared<hiberlite::Database>();
        auto hiberDb2 = std::any_cast<std::shared_ptr<hiberlite::Database>>(hiberDb);
		std::string totalProcessName = program_invocation_name; // Linux specific
		std::size_t processNamePos = totalProcessName.find_last_of("/\\");
		std::string justProcessName = totalProcessName.substr(processNamePos+1);
		std::ostringstream databaseName;
		databaseName << justProcessName << "Database.db";
        hiberDb2->open(":memory:"); // open(databaseName.str());
	}
    void CloseHiberlite()
	{
        auto hiberDb2 = std::any_cast<std::shared_ptr<hiberlite::Database>>(hiberDb);
		hiberDb2->close();
	}
public:
    UnitOfWorkFactory()
    {
        repositoryType_ = REPOSITORY_TYPE;
        if (repositoryType_ == RepositoryType::ORM)
        {
            OpenHiberlite();
        }
        else if (repositoryType_ == RepositoryType::ODM)
        {
            OpenRavenDB();   
        }
    }
    virtual ~UnitOfWorkFactory()
    {
        if (repositoryType_ == RepositoryType::ORM)
        {
            CloseHiberlite();
        }
    }

    std::unique_ptr<UnitOfWork> GetNewUnitOfWork()
	{
        if (repositoryType_ == RepositoryType::ODM)
        {
            auto doc_store2 = std::any_cast<std::shared_ptr<ravendb::client::documents::DocumentStore>>(doc_store);
            return std::make_unique<UnitOfWork>(doc_store2);
        }
        else if (repositoryType_ == RepositoryType::ORM)
        {
            auto hiberDb2 = std::any_cast<std::shared_ptr<hiberlite::Database>>(hiberDb);
            return std::make_unique<UnitOfWork>(hiberDb2);
        }
        else
        {
            return std::make_unique<UnitOfWork>(nullptr);
        }
	}

    std::any GetDataBasePtr()
    {
        if (repositoryType_ == RepositoryType::ODM)
        {
            return doc_store;
        }
        else
        {
            return hiberDb; // will be equal to nullptr for non-ORM
        }
    }
};

} // namespace UnitOfWork
