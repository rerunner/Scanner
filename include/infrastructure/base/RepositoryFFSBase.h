#pragma once

#include <iostream>
#include <vector>
#include <iterator>
#include <iostream>
#include <string>
#include <string_view>
#include <mutex>
#include <fstream>
#include <cstdio>
#include <nlohmann/json.hpp>
#include "IRepositoryBase.h"
#include "GenLogger.hpp"

using json = nlohmann::json;

template <typename RepositoryBaseType>
class RepositoryFFSBase : public IRepositoryBase<RepositoryBaseType>
{
private:
  inline static std::mutex repMtx;

  std::string DBProcessName()
  {
    std::string totalProcessName = program_invocation_name; // Linux specific
    std::size_t processNamePos = totalProcessName.find_last_of("/\\");
    std::string justProcessName = totalProcessName.substr(processNamePos+1);
    return justProcessName;
  }
  std::string DBEntityName()
  {
    std::string dbEntityName = typeid(RepositoryBaseType).name(); // every compiler does what it wants...

    int prefix = (dbEntityName.size() < 11)? 1 : 2;
    dbEntityName.erase(0, prefix); // Remove prefix numbers indicating length
    return dbEntityName;
  }
public:
  virtual ~RepositoryFFSBase(){}
  
  void Store(RepositoryBaseType entity)
  {
    std::scoped_lock lock{repMtx};
    json jEntity = entity;
    std::ostringstream constructedFileName;
    constructedFileName << DBProcessName() << "_" << DBEntityName() << "_";
    constructedFileName << entity.GetId().Get();
    std::ofstream myfile;
    myfile.open (constructedFileName.str().c_str());
    myfile << jEntity.dump();
    myfile.close();
  }
	
  void Delete(RepositoryBaseType entity)
  {
    std::scoped_lock lock{repMtx};
    std::ostringstream constructedFileName;
    constructedFileName << DBProcessName() << "_" << DBEntityName() << "_";
    constructedFileName << entity.GetId().Get();
    std::remove(constructedFileName.str().c_str());
  }
	
  RepositoryBaseType Get(Uuid requestedId)
  {
    std::scoped_lock lock{repMtx};
    std::ostringstream constructedFileName;
    constructedFileName << DBProcessName() << "_" << DBEntityName() << "_";
    constructedFileName << requestedId.Get();

    std::ifstream f(constructedFileName.str().c_str());
    json data = json::parse(f);

    auto entity = data.template get<RepositoryBaseType>();

    return entity;
  }

  std::vector<RepositoryBaseType> GetAll()
  {
    std::scoped_lock lock{repMtx};
    std::vector<std::string> strList;
    std::vector<RepositoryBaseType> vList;

    std::string path = ".";
    // Make list of all files created for this entity type
    for (const auto & entry : std::filesystem::directory_iterator(path))
    {
        std::string entryName = entry.path();

        std::size_t foundProcessName = entryName.find(DBProcessName()); // Make sure to limit the files to this process
        if ((foundProcessName > 0) && (foundProcessName < 256))
        {
          entryName.erase(0, foundProcessName); // Remove ./ path
          std::size_t foundEntityName = entryName.find(DBEntityName());
          if ((foundEntityName > 0) && (foundEntityName < 256))
          {
              strList.push_back(entryName);
          }
        }
    }
    // Read all files in the resulting list
    // Convert json to entity and add to list
    if (strList.size() > 0)
    {
      for (const auto & iterFileName : strList)
      {
          std::string entryName = iterFileName;
          std::ifstream f(entryName.c_str());
          json data = json::parse(f);
          auto entity = data.template get<RepositoryBaseType>();
          vList.push_back(entity);
      }
    }
    return vList;
  }

  std::vector<RepositoryBaseType> GetAllChildren(Uuid parentId)
  {
    std::vector<RepositoryBaseType> vList;
    vList = GetAll();
    std::scoped_lock lock{repMtx};
    std::vector<RepositoryBaseType> finalList;
    
    if (vList.size() == 0)
    {
      GSL::Dprintf(GSL::ERROR, "GetAllChildren got empty list from GetAll");
    }
    else
    {
      for (auto &iter:vList)
      {
        if (parentId.Get().compare(iter.GetParentId().Get()) == 0)
        {
          GSL::Dprintf(GSL::DEBUG, "GetAllChildren found matching parentid!!!");
          finalList.push_back(iter);
        }
      }    
    }
    if (finalList.size() == 0)
    {
      GSL::Dprintf(GSL::ERROR, "GetAllChildren returns empty list");
    }
    return finalList;
  }
};
