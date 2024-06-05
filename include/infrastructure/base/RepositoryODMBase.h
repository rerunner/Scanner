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
class RepositoryODMBase : public IRepositoryBase<RepositoryBaseType>
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
public:
  virtual ~RepositoryODMBase(){}
  
  void Store(RepositoryBaseType entity)
  {
    std::scoped_lock lock{repMtx};
    json jEntity = entity;
    std::string entityName = typeid(RepositoryBaseType).name();
    entityName.erase(0, 2); // Remove numbers
    std::ostringstream constructedFileName;
    constructedFileName << DBProcessName() << "_" << entityName << "_";
    constructedFileName << entity.GetId().Get();
    std::ofstream myfile;
    myfile.open (constructedFileName.str().c_str());
    myfile << jEntity.dump();
    myfile.close();
  }
	
  void Delete(RepositoryBaseType entity)
  {
    std::scoped_lock lock{repMtx};
    std::string entityName = typeid(RepositoryBaseType).name();
    entityName.erase(0, 2); // Remove numbers
    std::ostringstream constructedFileName;
    constructedFileName << DBProcessName() << "_" << entityName << "_";
    constructedFileName << entity.GetId().Get();
    std::remove(constructedFileName.str().c_str());
  }
	
  RepositoryBaseType Get(Uuid requestedId)
  {
    std::scoped_lock lock{repMtx};
    std::string entityName = typeid(RepositoryBaseType).name();
    entityName.erase(0, 2); // Remove numbers
    std::ostringstream constructedFileName;
    constructedFileName << DBProcessName() << "_" << entityName << "_";
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

    std::string entityName = typeid(RepositoryBaseType).name();
    entityName.erase(0, 2); // Remove numbers

    std::string path = ".";
    // Make list of all files created for this entity type
    for (const auto & entry : std::filesystem::directory_iterator(path))
    {
        //std::cout << entry.path() << std::endl;
        std::string entryName = entry.path();
        if (entryName.find(entityName))
        {
            strList.push_back(entry.path());
        }
    }
    // Read all files in the resulting list
    // Convert json to entity and add to list
    for (const auto & iterFileName : strList)
    {
        std::string entryName = iterFileName;
        std::ifstream f(entryName.c_str());
        json data = json::parse(f);
        auto entity = data.template get<RepositoryBaseType>();
        vList.push_back(entity);
    }

    return vList;
  }

  std::vector<RepositoryBaseType> GetAllChildren(Uuid parentId)
  {
    std::scoped_lock lock{repMtx};
    std::vector<RepositoryBaseType> vList;
    std::vector<RepositoryBaseType> finalList;

    vList = GetAll();

    for (auto &iter:vList)
    {
      if (parentId.Get().compare(iter.GetParentId().Get()) == 0)
      {
        finalList.push_back(iter);
      }
    }    

    return finalList;
  }
};
