#ifndef UUID_GENERATOR_H
#define UUID_GENERATOR_H

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

class UuidGenerator
{
public:
  static boost::uuids::uuid generateUuid()
  {
    static boost::uuids::random_generator idGenerator;
    return idGenerator();
  }
};

#endif // UUID_GENERATOR_H
