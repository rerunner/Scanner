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

    void Commit()
    {
        std::unique_ptr<IRepositoryFactory<EntityType>> repositoryFactory = std::make_unique<RepositoryFactory<EntityType>>();
        auto repository = repositoryFactory->GetRepository(RepositoryType::ORM);
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
    EntityRegister(std::shared_ptr<EntityType> newEnt, RegistryTypeEnum newRegistryType)
    {
        entityInstance = newEnt;
        registryType = newRegistryType;
    }
    
    ~EntityRegister()
    {
        Commit();
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

public:
    UnitOfWork()
    {
        GSL::Dprintf(GSL::DEBUG, "UnitOfWork created. UoW ID = ", _context.Get());
    }
    virtual ~UnitOfWork()
    {
        GSL::Dprintf(GSL::DEBUG, "UnitOfWork destroyed. UoW ID = ", _context.Get());
    }

    template <typename EntityType>
    void RegisterNew(std::shared_ptr<EntityType> entPtr)
    {
        GSL::Dprintf(GSL::DEBUG, "ENTER");
        EntityRegisterPtr<EntityType> myNewEntityPtr = std::make_shared<EntityRegister<EntityType>>(entPtr, RegistryTypeEnum::RegisterNew);
        _newEntities.push_back(std::move(myNewEntityPtr)); //Register
        GSL::Dprintf(GSL::DEBUG, "EXIT");
    }

    template <typename EntityType>
    void RegisterDirty(std::shared_ptr<EntityType> entPtr)
    { 
        GSL::Dprintf(GSL::DEBUG, "ENTER");
        EntityRegisterPtr<EntityType> myUpdatedEntityPtr = std::make_shared<EntityRegister<EntityType>>(entPtr, RegistryTypeEnum::RegisterDirty);
        _updatedEntities.push_back(std::move(myUpdatedEntityPtr)); //Register
        GSL::Dprintf(GSL::DEBUG, "EXIT");
    }

    template <typename EntityType>
    void RegisterDeleted(std::shared_ptr<EntityType> entPtr)
    { 
        GSL::Dprintf(GSL::DEBUG, "ENTER");
        EntityRegisterPtr<EntityType> myDeletedEntityPtr = std::make_shared<EntityRegister<EntityType>>(entPtr, RegistryTypeEnum::RegisterDeleted);
        _updatedEntities.push_back(std::move(myDeletedEntityPtr)); //Register
        GSL::Dprintf(GSL::DEBUG, "EXIT");
    }
    
    void Commit()
    {
        GSL::Dprintf(GSL::DEBUG, "Commit UoW ID = ", _context.Get());
        // Too bad that we don't know the templated types anymore :-(
        // Committing changed or new objects happens in the destructor of list of changed entities (at destruction of this UoW instance)
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

} // namespace UnitOfWork