#ifndef ENTITY_BASE_H
#define ENTITY_BASE_H

#include <iostream>
#include "boost/lexical_cast.hpp"
#include "UuidGenerator.hpp"

class EntityBase
{
protected:
  std::string id_;
  
public:
  EntityBase()
  {
    id_ = boost::lexical_cast<std::string>(UuidGenerator::generateUuid());
  }
  
  std::string GetId() const { return id_; }

  bool operator==(const EntityBase& other) const
  {
    return (id_ == other.GetId());
  }

  bool operator!=(const EntityBase& other) const
  {
    return !(*this == other);
  }
};

#endif /* ENTITY_BASE_H */
  
