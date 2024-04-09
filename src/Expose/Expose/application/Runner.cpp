
#include "Expose/application/Runner.hpp"
#include "Expose/application/AppComponent.hpp"
#include "Expose/application/ExposeController.hpp"

#include "oatpp-swagger/AsyncController.hpp"
#include "oatpp/web/server/AsyncHttpConnectionHandler.hpp"
#include "oatpp/network/Server.hpp"

namespace Expose { namespace Application {

Runner::Runner() {
  OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router, Qualifiers::SERVICE_EXPOSE);

  oatpp::web::server::api::Endpoints docEndpoints;

  /* Add ExposeApiController */
  docEndpoints.append(router->addController(std::make_shared<controller::ExposeController>())->getEndpoints());

  OATPP_COMPONENT(std::shared_ptr<oatpp::swagger::DocumentInfo>, documentInfo, Qualifiers::SERVICE_EXPOSE);
  OATPP_COMPONENT(std::shared_ptr<oatpp::swagger::Resources>, resources, Qualifiers::SERVICE_EXPOSE);

  router->addController(oatpp::swagger::AsyncController::createShared(docEndpoints, documentInfo, resources));
}

void Runner::run(std::list<std::thread>& acceptingThreads) {
  /* Get router component */
  OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router, Qualifiers::SERVICE_EXPOSE);

  /* Create connection handler */
  auto connectionHandler = oatpp::web::server::AsyncHttpConnectionHandler::createShared(router);

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
