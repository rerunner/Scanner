#pragma once

#include "hiberlite.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include "boost/lexical_cast.hpp"
#include <boost/uuid/uuid_io.hpp>
#include "domain/base/ValueObjectBase.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Uuid : public ValueObjectBase
{
private:
  std::string uuid_;
  static boost::uuids::uuid generateUuid()
  {
    static boost::uuids::random_generator idGenerator;
    return idGenerator();
  }
  //Boilerplate start
  friend class hiberlite::access;
  template<class Archive>
  void hibernate(Archive & ar)
  {
    ar & HIBERLITE_NVP(uuid_);
  }
  //Boilerplate end
public:
  Uuid ();
  Uuid (std::string withUuid_);
  virtual bool operator==(const ValueObjectBase& other) const override;
  const std::string Get() const;

  //JSON boilerplate
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Uuid, uuid_)
};
