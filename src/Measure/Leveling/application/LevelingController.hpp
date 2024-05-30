#pragma once

#include "Leveling/application/Constants.hpp"

// OATPP
#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"

#include "Leveling/application/Leveling.hpp"
#include "Leveling/application/LevelingCommand.hpp"

#include "GenLogger.hpp"

namespace Leveling { namespace Application { namespace controller {

//static Leveling myLeveling; 
static LevelingCommands::CommandExecutor *executor;
static std::shared_ptr<LevelingCommands::Command> command;

class LevelingController : public oatpp::web::server::api::ApiController 
{
  private:
  Leveling myLeveling;
  public:
  LevelingController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper, Qualifiers::SERVICE_LEVELING) /* Inject object mapper */)
      : oatpp::web::server::api::ApiController(objectMapper) 
      {
        executor = new LevelingCommands::CommandExecutor{myLeveling};
      }
  ~LevelingController()
  {
    if (executor)
    {
      delete executor;
      executor = NULL;
    }
  }

  public:
  static std::shared_ptr<LevelingController> createShared(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
  {
    return std::shared_ptr<LevelingController>(new LevelingController(objectMapper));
  }

#include OATPP_CODEGEN_BEGIN(ApiController) ///< Begin codegen section

  ENDPOINT_INFO(measureWafer) {
    // general
    info->summary = "measure heightmap of the wafer identified by waferId";
    info->addResponse<String>(Status::CODE_200, "application/json");
    info->addResponse<String>(Status::CODE_404, "text/plain");
    // params specific
    info->pathParams["waferId"].description = "Wafer Identifier";
  }

  ENDPOINT_ASYNC("PUT", "/measure/leveling/measure/{waferId}", measureWafer) {

    ENDPOINT_ASYNC_INIT(measureWafer)

    // Coroutine entrypoint act) returns Action (what to do next)
    Action act() override {
      GSL::Dprintf(GSL::DEBUG, "Give commands to leveling object");
      auto mywId = request->getPathVariable("waferId");
      std::string printableWaferId = mywId;
      GSL::Dprintf(GSL::DEBUG, "Leveling Command Received: Measure Heightmap of Wafer with uuid = ", printableWaferId);

      return request->readBodyToStringAsync().callbackTo(&measureWafer::returnResponse);
    }

    Action returnResponse(const oatpp::String& body){
      /* return Action to return created OutgoingResponse */
      GSL::Dprintf(GSL::DEBUG, "Give commands to leveling object");
      auto mywId = request->getPathVariable("waferId");
      std::string requestedWaferIdStr = mywId;
      Uuid requestedWaferId(requestedWaferIdStr);
      command = std::make_shared<LevelingCommands::Command>(LevelingCommands::MeasureWafer{requestedWaferId});
      GSL::Dprintf(GSL::DEBUG, "execute the command");
      std::visit(*executor, *command);  
      return _return(controller->createResponse(Status::CODE_200, "Measure heightmap requested\n"));
    }
  };

  ENDPOINT_ASYNC("GET", "/measure/leveling/hello", Root /* Name of the Coroutine */) {
    ENDPOINT_ASYNC_INIT(Root) ///< Generate constructor and default fields
    Action act() override {
      return _return(controller->createResponse(Status::CODE_200, "Hello from Leveling!\n"));
    }
  };

#include OATPP_CODEGEN_END(ApiController) //<-- codegen End

};

//#include OATPP_CODEGEN_END(ApiController) //<-- codegen End

}}} // namespaces
