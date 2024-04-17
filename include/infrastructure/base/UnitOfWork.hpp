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
#include "RepositoryORMBase.h"
#include "GenLogger.hpp"
#include "domain/WaferHeightMap.hpp"

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
        GSL::Dprintf(GSL::INFO, "Commit registry changes");
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
    UnitOfWork(){GSL::Dprintf(GSL::INFO, "UnitOfWork created. UoW ID = ", _context.Get());}
    virtual ~UnitOfWork(){GSL::Dprintf(GSL::INFO, "UnitOfWork destroyed. UoW ID = ", _context.Get());}

    template <typename EntityType>
    void RegisterNew(std::shared_ptr<EntityType> entPtr)
    {
        GSL::Dprintf(GSL::INFO, "ENTER");
        EntityRegisterPtr<EntityType> myNewEntityPtr = std::make_shared<EntityRegister<EntityType>>(std::move(entPtr), RegistryTypeEnum::RegisterNew);        
        _newEntities.push_back(std::move(myNewEntityPtr)); //Register
        GSL::Dprintf(GSL::INFO, "EXIT");
    }

    template <typename EntityType>
    void RegisterDirty(std::shared_ptr<EntityType> entPtr)
    { 
        GSL::Dprintf(GSL::INFO, "ENTER");
        EntityRegisterPtr<EntityType> myUpdatedEntityPtr = std::make_shared<EntityRegister<EntityType>>(std::move(entPtr), RegistryTypeEnum::RegisterDirty);
        _updatedEntities.push_back(std::move(myUpdatedEntityPtr)); //Register
        GSL::Dprintf(GSL::INFO, "EXIT");
    }

    template <typename EntityType>
    void RegisterDeleted(std::shared_ptr<EntityType> entPtr)
    { 
        GSL::Dprintf(GSL::INFO, "ENTER");
        EntityRegisterPtr<EntityType> myDeletedEntityPtr = std::make_shared<EntityRegister<EntityType>>(std::move(entPtr), RegistryTypeEnum::RegisterDeleted);
        _updatedEntities.push_back(std::move(myDeletedEntityPtr)); //Register
        GSL::Dprintf(GSL::INFO, "EXIT");
    }
    
    void Commit()
    {
        GSL::Dprintf(GSL::INFO, "Commit UoW ID = ", _context.Get());
        // Too bad that we don't know the templated types anymore :-(
        // Committing changed or new objects happens in the destructor of list of changed entities (at destruction of this UoW instance)
        _newEntities.clear(); // clearing calls destructor, hence commit happens
    }

    void Rollback()
    {
        //Todo, remove without commit
    }
    
    template <typename entityType>
    IRepositoryBase<entityType> *GetRepository()
    {
        GSL::Dprintf(GSL::INFO, "ENTER UoW ID = ", _context.Get());
        entityType dummyEntity;
        std::string typeKey = typeid(dummyEntity).name();
        Dict::iterator it = repositories_.find(typeKey);
        if (it != repositories_.end())
        {
            GSL::Dprintf(GSL::INFO, "Entity type ", typeKey, " found existing in UnitOfWork.");
            RepositoryORMBase<entityType> *repository = static_cast<RepositoryORMBase<entityType> *>(it->second); // Point to the correct repository object
            GSL::Dprintf(GSL::INFO, "EXIT UoW ID = ", _context.Get());
            return repository;
        }
        else // Create a new one
        {
            GSL::Dprintf(GSL::INFO, "Entity type ", typeKey, " not found yet in UnitOfWork, adding it.");
            // Create Factory for the Template type repository
            std::unique_ptr<IRepositoryFactory<entityType>> repositoryFactory = std::make_unique<RepositoryFactory<entityType>>();
            // Use factory to create specialized repository
            auto repository = repositoryFactory->GetRepository(RepositoryType::ORM);
            //repositories_[typeKey] = &repository;
            GSL::Dprintf(GSL::INFO, "EXIT UoW ID = ", _context.Get());
            return repository;
        }
    }

    template <typename entityType>
    entityType Get(Uuid id){/*repository->Get(id);*/ return nullptr;}

    template <typename entityType>
    std::list<entityType> GetAll(){/*todo*/}
};