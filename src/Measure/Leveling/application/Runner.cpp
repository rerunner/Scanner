
#include "Leveling/application/Runner.hpp"
#include "Leveling/application/AppComponent.hpp"
#include "Leveling/application/LevelingController.hpp"

#include "oatpp-swagger/AsyncController.hpp"
#include "oatpp/web/server/AsyncHttpConnectionHandler.hpp"
#include "oatpp/network/Server.hpp"

namespace Leveling { namespace Application {

Runner::Runner() {
  OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router, Qualifiers::SERVICE_LEVELING);

  oatpp::web::server::api::Endpoints docEndpoints;

  /* Add LevelingApiController */
  docEndpoints.append(router->addController(std::make_shared<controller::LevelingController>())->getEndpoints());

  OATPP_COMPONENT(std::shared_ptr<oatpp::swagger::DocumentInfo>, documentInfo, Qualifiers::SERVICE_LEVELING);
  OATPP_COMPONENT(std::shared_ptr<oatpp::swagger::Resources>, resources, Qualifiers::SERVICE_LEVELING);

  //router->addController(oatpp::swagger::Controller::createShared(docEndpoints, documentInfo, resources));
  router->addController(oatpp::swagger::AsyncController::createShared(docEndpoints, documentInfo, resources));
}

void Runner::run(std::list<std::thread>& acceptingThreads) {
  /* Get router component */
  OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router, Qualifiers::SERVICE_LEVELING);

  /* Create connection handler */
  //auto connectionHandler = oatpp::web::server::HttpConnectionHandler::createShared(router);
  auto connectionHandler = oatpp::web::server::AsyncHttpConnectionHandler::createShared(router);

  acceptingThreads.push_back(std::thread([router, connectionHandler]{
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider, Qualifiers::SERVICE_LEVELING);
    oatpp::network::Server server(connectionProvider, connectionHandler);
    OATPP_LOGi("measure-service", "server is listening on port '%s'", (const char*)connectionProvider->getProperty("port").getData());
    server.run();
  }));

  acceptingThreads.push_back(std::thread([router, connectionHandler]{
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider, Qualifiers::SERVICE_LEVELING_VH);
    oatpp::network::Server server(connectionProvider, connectionHandler);
    OATPP_LOGi("leveling-service", "server is listening on virtual interface '%s'", (const char*)connectionProvider->getProperty("host").getData());
    server.run();
  }));

}

}}
