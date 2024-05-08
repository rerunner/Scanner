#pragma once

#include <vector>

template <typename T>
class IRepositoryBase
{
public:
  virtual void Store(T entity) = 0;
  virtual void Delete(T entity) = 0;
  virtual T Get(Uuid id) = 0;
  virtual std::vector<T> GetAll() = 0;
  virtual std::vector<T> GetAllChildren(Uuid id) = 0;
};
