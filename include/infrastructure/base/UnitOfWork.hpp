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
#include <boost/any.hpp>
#include "Uuid.hpp"
#include "IRepositoryFactory.h"
#include "IRepositoryBase.h"
#include "RepositoryFactory.h" // ?
#include "RepositoryORMBase.h"
#include "GenLogger.hpp"
#include "domain/WaferHeightMap.hpp"

//#define REPOSITORY_TYPE RepositoryType::HeapRepository
#define REPOSITORY_TYPE RepositoryType::ORM

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
    hiberlite::Database *db;
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
    EntityRegister(std::shared_ptr<EntityType> newEnt, RegistryTypeEnum newRegistryType, hiberlite::Database *passedDb)
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
    std::list<boost::any> _newEntities;
    std::list<boost::any> _updatedEntities;
    std::list<boost::any> _deletedEntities;
    hiberlite::Database *db;
    RepositoryType repositoryType_;

public:
    UnitOfWork(hiberlite::Database *passedDb)
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
            try {
                db->registerBeanClass<EntityType>();
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
            try {
                db->registerBeanClass<EntityType>();
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
            try {
                db->registerBeanClass<EntityType>();
            }
            catch (std::exception& e) {
                GSL::Dprintf(GSL::DEBUG, "didn't register beanclass: ", e.what());
            }
        }
		
        EntityRegisterPtr<EntityType> myDeletedEntityPtr = std::make_shared<EntityRegister<EntityType>>(entPtr, RegistryTypeEnum::RegisterDeleted, db);
        _updatedEntities.push_back(std::move(myDeletedEntityPtr)); //Register
        GSL::Dprintf(GSL::DEBUG, "EXIT");
    }
    
    void Commit()
    {
        GSL::Dprintf(GSL::DEBUG, "Commit UoW ID = ", _context.Get());
        // Too bad that we don't know the templated types anymore :-(
        // Committing changed or new objects happens in the destructor of list of changed entities (at destruction of this UoW instance)
        if (repositoryType_ == RepositoryType::ORM)
        {
            try {
                db->createModel();
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
    hiberlite::Database *db;
    RepositoryType repositoryType_;
    void OpenHiberlite()
	{
		db = new hiberlite::Database;
		std::string totalProcessName = program_invocation_name; // Linux specific
		std::size_t processNamePos = totalProcessName.find_last_of("/\\");
		std::string justProcessName = totalProcessName.substr(processNamePos+1);
		std::ostringstream databaseName;
		databaseName << justProcessName << "Database.db";
		GSL::Dprintf(GSL::DEBUG, "Opening ", databaseName.str());
		//db->open(databaseName.str());
        db->open(":memory:");
	}
    void CloseHiberlite()
	{
		GSL::Dprintf(GSL::DEBUG, "Closing database");
		db->close();
		delete db;
		db = nullptr;
	}
public:
    UnitOfWorkFactory()
    {
        repositoryType_ = REPOSITORY_TYPE;
        if (repositoryType_ == RepositoryType::ORM)
        {
            OpenHiberlite();
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
        return std::make_unique<UnitOfWork>(db);
	}
    hiberlite::Database *GetDataBasePtr()
    {
        return db;
    }
};

} // namespace UnitOfWork