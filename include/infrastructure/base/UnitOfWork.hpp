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


template < typename EntityType> 
class EntityRegister
{
    std::shared_ptr<EntityType> entityInstance;

    void Commit()
    {
        GSL::Dprintf(GSL::INFO, "Commiter start");
        std::unique_ptr<IRepositoryFactory<EntityType>> repositoryFactory = std::make_unique<RepositoryFactory<EntityType>>();
        auto repository = repositoryFactory->GetRepository(RepositoryType::ORM);
        repository->Store(*entityInstance);
        GSL::Dprintf(GSL::INFO, "Commiter end");
    }
public:
    EntityRegister(std::shared_ptr<EntityType> newEnt){entityInstance = newEnt;}
    
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
    void RegisterNew(EntityType entity)
    {
        GSL::Dprintf(GSL::INFO, "ENTER");

        std::shared_ptr<EntityType> entPtr = std::make_shared<EntityType>(entity); //shared pointer copy of entity
        EntityRegisterPtr<EntityType> myNewEntityPtr = std::make_shared<EntityRegister<EntityType>>(std::move(entPtr));
        
        _newEntities.push_back(std::move(myNewEntityPtr)); //Register
        
        GSL::Dprintf(GSL::INFO, "EXIT");
    }

    template <typename EntityType>
    void RegisterDirty(EntityType entity)
    { 
        GSL::Dprintf(GSL::INFO, "ENTER");

        std::shared_ptr<EntityType> entPtr = std::make_shared<EntityType>(entity); //shared pointer copy of entity
        EntityRegisterPtr<EntityType> myUpdatedEntityPtr = std::make_shared<EntityRegister<EntityType>>(std::move(entPtr));
        
        _updatedEntities.push_back(std::move(myUpdatedEntityPtr)); //Register
        
        GSL::Dprintf(GSL::INFO, "EXIT");
    }

    template <typename EntityType>
    void RegisterDeleted(EntityType entity)
    { 
        GSL::Dprintf(GSL::INFO, "ENTER");

        std::shared_ptr<EntityType> entPtr = std::make_shared<EntityType>(entity); //shared pointer copy of entity
        EntityRegisterPtr<EntityType> myDeletedEntityPtr = std::make_shared<EntityRegister<EntityType>>(std::move(entPtr));
        
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
        //Todo, remove after clearing
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
