#pragma once

#include <string>
#include <list>
#include "Uuid.hpp"
#include "IRepositoryFactory.h"
#include "IRepositoryBase.h"

class IUnitOfWork
{
    public:
    virtual ~IUnitOfWork(){}

    // Change notification methods.
    template <typename T> 
    void RegisterNew(T){}
    template <typename T> 
    void RegisterDirty(T){}
    template <typename T> 
    void RegisterClean(T){}
    template <typename T> 
    void RegisterDeleted(T){} // RegisterDeleted
    
    // Commit and rollback methods.
    virtual void Commit() = 0;  // Store all changes
    virtual void Rollback() = 0;

    template <typename T> 
    T Get(Uuid id){return T;}
    template <typename T> 
    std::list<T> GetAll(Uuid id){return;}
    
    virtual IRepositoryBase<T>* GetRepository<T>(RepositoryType repository) = 0;
};