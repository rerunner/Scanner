#ifndef VALUE_OBJECT_BASE_H
#define VALUE_OBJECT_BASE_H

#include <iostream>

class ValueObjectBase
{
public:
  virtual bool operator==(const ValueObjectBase& other) const = 0;

  virtual bool operator!=(const ValueObjectBase& other) const
  {
    return !(*this == other);
  }

  virtual ~ValueObjectBase() = default;
};

#endif /* VALUE_OBJECT_BASE_H */

