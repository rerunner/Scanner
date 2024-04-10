#pragma once

#include "Expose/application/Constants.hpp"

// OATPP
#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"

#include "Expose/application/Expose.hpp"
#include "Expose/application/ExposeCommand.hpp"

#include "GenLogger.hpp"

namespace Expose { namespace Application { namespace controller {

static Application::Expose myExpose; 
static ExposeCommands::CommandExecutor *executor;
static std::shared_ptr<ExposeCommands::Command> command; 

class ExposeController : public oatpp::web::server::api::ApiController 
{
  public:
  ExposeController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper, Qualifiers::SERVICE_EXPOSE) /* Inject object mapper */)
      : oatpp::web::server::api::ApiController(objectMapper) 
      {
        executor = new ExposeCommands::CommandExecutor{myExpose};
      }
  ~ExposeController()
  {
    if (executor)
    {
      delete executor;
      executor = NULL;
    }
  }

  public:
  static std::shared_ptr<ExposeController> createShared(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
  {
    return std::shared_ptr<ExposeController>(new ExposeController(objectMapper));
  }

#include OATPP_CODEGEN_BEGIN(ApiController) ///< Begin codegen section

  ENDPOINT_INFO(exposeWafer) {
    // general
    info->summary = "expose wafer identified by waferId";
    info->addResponse<String>(Status::CODE_200, "application/json");
    info->addResponse<String>(Status::CODE_404, "text/plain");
    // params specific
    info->pathParams["waferId"].description = "Wafer Identifier";
  }

  ENDPOINT_ASYNC("PUT", "/expose/expose/{waferId}", exposeWafer) {

    ENDPOINT_ASYNC_INIT(exposeWafer)

    // Coroutine entrypoint act) returns Action (what to do next)
    Action act() override {
      GSL::Dprintf(GSL::INFO, "Give commands to expose object");
      auto mywId = request->getPathVariable("waferId");
      std::string printableWaferId = mywId;
      GSL::Dprintf(GSL::INFO, "Expose Command Received: Expose Wafer with uuid = ", printableWaferId);

      return request->readBodyToStringAsync().callbackTo(&exposeWafer::returnResponse);
    }

    Action returnResponse(const oatpp::String& body){
      /* return Action to return created OutgoingResponse */
      GSL::Dprintf(GSL::INFO, "Give commands to expose object");
      auto mywId = request->getPathVariable("waferId");
      std::string requestedWaferId = mywId;
      command = std::make_shared<ExposeCommands::Command>(ExposeCommands::ExposeWafer{requestedWaferId});
      GSL::Dprintf(GSL::INFO, "execute the command");
      std::visit(*executor, *command);  
      return _return(controller->createResponse(Status::CODE_200, "Expose wafer completed\n"));
    }
  };

#include OATPP_CODEGEN_END(ApiController) //<-- codegen End

}; // class end



}}} // namespaces
