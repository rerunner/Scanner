#pragma once

template <typename T>
class IRepositoryBase
{
public:
  virtual void Store(T entity) = 0;
  virtual void Delete(T entity) = 0;
  virtual T Get(std::string id) = 0;
};
