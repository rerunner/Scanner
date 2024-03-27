#pragma once

#include "application/Constants.hpp"

// OATPP
#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"

#include "application/Expose.hpp"
#include "application/ExposeCommand.hpp"

namespace Expose { namespace Application { namespace controller {

#include OATPP_CODEGEN_BEGIN(ApiController) ///< Begin codegen section

class ExposeController : public oatpp::web::server::api::ApiController 
{
  private:
  // Inject Expose component
  //OATPP_COMPONENT(std::shared_ptr<Application::Expose>, m_expose, Qualifiers::SERVICE_EXPOSE);
  Application::Expose myExpose; 

  public:
  ExposeController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper, Qualifiers::SERVICE_EXPOSE) /* Inject object mapper */)
      : oatpp::web::server::api::ApiController(objectMapper) {}

  public:

  ENDPOINT_INFO(exposeWafer) {
    // general
    info->summary = "expose wafer identified by waferId";
    info->addResponse<String>(Status::CODE_200, "application/json");
    info->addResponse<String>(Status::CODE_404, "text/plain");
    // params specific
    info->pathParams["waferId"].description = "Wafer Identifier";
  }
  ENDPOINT("PUT", "/expose/{waferId}", exposeWafer,
           PATH(String, waferId)) {
    std::cout << "Give commands to expose object" << std::endl;
    ExposeCommands::CommandExecutor executor(myExpose); 
    // actual execution, can be moved to queue command processor
    ExposeCommands::Command command; 
    std::cout << "Command: Expose Wafer with uuid" << std::endl;
    command = ExposeCommands::ExposeWafer{waferId}; 
    std::cout << "execute the command" << std::endl;
    std::visit(executor, command);  
    return createResponse(Status::CODE_200, "Exposure started!");
  }
};

#include OATPP_CODEGEN_END(ApiController) //<-- codegen End

}}} // namespaces
