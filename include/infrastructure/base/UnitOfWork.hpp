#pragma once

#include "IUnitOfWork.hpp"

template <typename T>
class UnitOfWork : IUnitOfWork
{
private:
    Uuid _context;
    std::unique_ptr<IRepositoryFactory<T>> repositoryFactory;
    // Soon std::shared_ptr<IRepositoryBase<T>> repository;
    IRepositoryBase<T> *repository;
    std::vector<T> newEntities; // Vector holding all new objects linked to a Unit Of Work
    std::vector<T> updatedEntities; // Vector holding all changed objects linked to a Unit Of Work
    std::vector<T> deletedEntities; // Vector holding all deleted objects linked to a Unit Of Work

public:
    virtual ~UnitOfWork<T>(){}

    virtual void RegisterNew(T entity){ newEntities.push_back(entity); }
    virtual void RegisterDirty(T entity){ updatedEntities.push_back(entity); }
    virtual void RegisterDeleted(T entity){ deletedEntities.push_back(entity); }

    virtual void Commit()
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

    virtual void Rollback()
    {
        newEntities.clear(); 
        updatedEntities.clear(); 
        deletedEntities.clear(); 
    }
    
    virtual IRepositoryBase<T>* GetRepository(RepositoryType repository)
    {
        // Create Factory for the Template type repository
        repositoryFactory = std::make_shared<RepositoryFactory<T>>;
        // Use factory to create specialized repository
        repository = repositoryFactory->GetRepository(RepositoryType::ORM); // or e.g. RepositoryType::HeapRepository
        return repository;
    }

    virtual T Get(Uuid id){repository->Get(id);}
    virtual std::list<T> GetAll(){/*todo*/}
};
