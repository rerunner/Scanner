#pragma once

#include "Leveling/application/Constants.hpp"

// OATPP
#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"

#include "Leveling/application/Leveling.hpp"
#include "Leveling/application/LevelingCommand.hpp"

namespace Leveling { namespace Application { namespace controller {

#include OATPP_CODEGEN_BEGIN(ApiController) ///< Begin codegen section

class LevelingController : public oatpp::web::server::api::ApiController 
{
  private:
  // Inject Leveling component
  //OATPP_COMPONENT(std::shared_ptr<Application::Leveling>, m_leveling, Qualifiers::SERVICE_LEVELING);
  Leveling myLeveling; 

  public:
  LevelingController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper, Qualifiers::SERVICE_LEVELING) /* Inject object mapper */)
      : oatpp::web::server::api::ApiController(objectMapper) {}

  public:

  ENDPOINT_INFO(measureWafer) {
    // general
    info->summary = "measure heightmap of the wafer identified by waferId";
    info->addResponse<String>(Status::CODE_200, "application/json");
    info->addResponse<String>(Status::CODE_404, "text/plain");
    // params specific
    info->pathParams["waferId"].description = "Wafer Identifier";
  }
  ENDPOINT("PUT", "/measure/leveling/{waferId}", measureWafer,
           PATH(String, waferId)) {
    std::cout << "Give commands to leveling object" << std::endl;
    LevelingCommands::CommandExecutor executor(myLeveling); 
    // actual execution, can be moved to queue command processor
    LevelingCommands::Command command; 
    std::string mywId = waferId;
    std::cout << "Command: Measure Heightmap of Wafer with uuid = " << mywId << std::endl;
    command = LevelingCommands::MeasureWafer{waferId}; 
    std::cout << "execute the command" << std::endl;
    std::visit(executor, command);  
    return createResponse(Status::CODE_200, "Heightmap measurement started!");
  }
};

#include OATPP_CODEGEN_END(ApiController) //<-- codegen End

}}} // namespaces
