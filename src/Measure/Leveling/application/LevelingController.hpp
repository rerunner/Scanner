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

//#include OATPP_CODEGEN_BEGIN(ApiController) ///< Begin codegen section

static Leveling myLeveling; 
static LevelingCommands::CommandExecutor *executor;
static std::shared_ptr<LevelingCommands::Command> command; 

class LevelingController : public oatpp::web::server::api::ApiController 
{
  public:
  LevelingController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper, Qualifiers::SERVICE_LEVELING) /* Inject object mapper */)
      : oatpp::web::server::api::ApiController(objectMapper) {}
  //protected:
  //LevelingController(const std::shared_ptr<ObjectMapper>& objectMapper)
  //: oatpp::web::server::api::ApiController(objectMapper) {}

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
#if 0 // SYNC vs ASYNC
  ENDPOINT("PUT", "/measure/leveling/measure/{waferId}", measureWafer,
           PATH(String, waferId)) {
    std::cout << "Give commands to leveling object" << std::endl;
    //LevelingCommands::CommandExecutor executor(myLeveling); 
    //executor = std::make_unique<LevelingCommands::CommandExecutor>(LevelingCommands::CommandExecutor{myLeveling});
    executor = new LevelingCommands::CommandExecutor{myLeveling}; // TODO FIX MEMORY LEAK!!!
    // actual execution, can be moved to queue command processor
    std::string mywId = waferId;
    std::cout << "Command: Measure Heightmap of Wafer with uuid = " << mywId << std::endl;
    command = std::make_unique<LevelingCommands::Command>(LevelingCommands::MeasureWafer{waferId});
    std::cout << "execute the command" << std::endl;
    std::visit(*executor, *command);  
    return createResponse(Status::CODE_200, "Heightmap measurement completed!");
  }
#else
  ENDPOINT_ASYNC("PUT", "/measure/leveling/measure/{waferId}", measureWafer) {

    ENDPOINT_ASYNC_INIT(measureWafer)

    // Coroutine entrypoint act) returns Action (what to do next)
    Action act() override {
      std::cout << "Give commands to leveling object" << std::endl;
      auto mywId = request->getPathVariable("waferId");
      std::string printableWaferId = mywId;
      std::cout << "Leveling Command Received: Measure Heightmap of Wafer with uuid = " << printableWaferId << std::endl;

      //return _return(controller->createResponse(Status::CODE_200, "Heightmap measurement completed!"));
      return request->readBodyToStringAsync().callbackTo(&measureWafer::returnResponse);
      }

    Action returnResponse(const oatpp::String& body){
      /* return Action to return created OutgoingResponse */
      std::cout << "Give commands to leveling object" << std::endl;
      auto mywId = request->getPathVariable("waferId");
      std::string requestedWaferId = mywId;
      executor = new LevelingCommands::CommandExecutor{myLeveling}; // TODO FIX MEMORY LEAK!!!
      command = std::make_shared<LevelingCommands::Command>(LevelingCommands::MeasureWafer{requestedWaferId});
      std::cout << "execute the command" << std::endl;
      std::visit(*executor, *command);  
      //return _return(controller->createResponse(Status::CODE_200, body));
      return _return(controller->createResponse(Status::CODE_200, "Heightmap measurement completed!"));
    }
  };
#endif
#include OATPP_CODEGEN_END(ApiController) //<-- codegen End

};

//#include OATPP_CODEGEN_END(ApiController) //<-- codegen End

}}} // namespaces
