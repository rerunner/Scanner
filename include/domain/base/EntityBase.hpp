#ifndef ENTITY_BASE_H
#define ENTITY_BASE_H

#include <iostream>
#include "boost/lexical_cast.hpp"
#include "Uuid.hpp"

class EntityBase
{
protected:
  Uuid id_;
  Uuid parentId_;
  
public:
  EntityBase(){}
  
  Uuid GetId() const { return id_; }

  Uuid GetParentId() const { return parentId_; }

  bool operator==(const EntityBase& other) const
  {
    return (id_.Get() == other.GetId().Get());
  }

  bool operator!=(const EntityBase& other) const
  {
    return !(*this == other);
  }
};

#endif /* ENTITY_BASE_H */
  
