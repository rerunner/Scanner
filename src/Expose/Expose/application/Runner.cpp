
#include "application/Runner.hpp"
#include "application/AppComponent.hpp"
#include "application/ExposeController.hpp"

#include "oatpp-swagger/Controller.hpp"
#include "oatpp/web/server/HttpConnectionHandler.hpp"
#include "oatpp/network/Server.hpp"

namespace Expose { namespace Application {

Runner::Runner() {
  OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router, Qualifiers::SERVICE_EXPOSE);

  oatpp::web::server::api::Endpoints docEndpoints;

  /* Add ExposeApiController */
  docEndpoints.append(router->addController(std::make_shared<controller::ExposeController>())->getEndpoints());

  OATPP_COMPONENT(std::shared_ptr<oatpp::swagger::DocumentInfo>, documentInfo, Qualifiers::SERVICE_EXPOSE);
  OATPP_COMPONENT(std::shared_ptr<oatpp::swagger::Resources>, resources, Qualifiers::SERVICE_EXPOSE);

  router->addController(oatpp::swagger::Controller::createShared(docEndpoints, documentInfo, resources));
}

void Runner::run(std::list<std::thread>& acceptingThreads) {
  /* Get router component */
  OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router, Qualifiers::SERVICE_EXPOSE);

  /* Create connection handler */
  auto connectionHandler = oatpp::web::server::HttpConnectionHandler::createShared(router);

  acceptingThreads.push_back(std::thread([router, connectionHandler]{
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider, Qualifiers::SERVICE_EXPOSE);
    oatpp::network::Server server(connectionProvider, connectionHandler);
    OATPP_LOGI("expose-service", "server is listening on port '%s'", (const char*)connectionProvider->getProperty("port").getData());
    server.run();
  }));

  acceptingThreads.push_back(std::thread([router, connectionHandler]{
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider, Qualifiers::SERVICE_EXPOSE_VH);
    oatpp::network::Server server(connectionProvider, connectionHandler);
    OATPP_LOGI("expose-service", "server is listening on virtual interface '%s'", (const char*)connectionProvider->getProperty("host").getData());
    server.run();
  }));

}

}}
