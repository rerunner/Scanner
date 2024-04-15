#pragma once

#include <string>
#include <list>
#include "Uuid.hpp"
#include "IRepositoryFactory.h"
#include "IRepositoryBase.h"

template <typename T>
class UnitOfWork
{
private:
    Uuid _context;
    std::unique_ptr<IRepositoryFactory<T>> repositoryFactory;
    IRepositoryBase<T> *repository;
    std::vector<T> newEntities; // Vector holding all new objects linked to a Unit Of Work
    std::vector<T> updatedEntities; // Vector holding all changed objects linked to a Unit Of Work
    std::vector<T> deletedEntities; // Vector holding all deleted objects linked to a Unit Of Work

public:
    UnitOfWork<T>()
    {
        // Create Factory for the Template type repository
        repositoryFactory = std::make_unique<RepositoryFactory<T>>();
        // Use factory to create specialized repository
        repository = repositoryFactory->GetRepository(RepositoryType::ORM);
    }
    virtual ~UnitOfWork<T>(){}

    void RegisterNew(T entity){ newEntities.push_back(entity); }
    void RegisterDirty(T entity){ updatedEntities.push_back(entity); }
    void RegisterDeleted(T entity){ deletedEntities.push_back(entity); }

    void Commit()
    {
        for (auto &iter:newEntities)
        {
            repository->Store(iter);
        }
        newEntities.clear(); 
        for (auto &iter:updatedEntities)
        {
            repository->Store(iter);
        }
        updatedEntities.clear(); 
        for (auto &iter:deletedEntities)
        {
            repository->Delete(iter);
        }
        deletedEntities.clear(); 
    }

    void Rollback()
    {
        newEntities.clear(); 
        updatedEntities.clear(); 
        deletedEntities.clear(); 
    }
    
    template <typename entityType>
    IRepositoryBase<entityType> *GetRepository()
    {
        return repository;
    }

    T Get(Uuid id){repository->Get(id);}
    std::list<T> GetAll(){/*todo*/}
};
