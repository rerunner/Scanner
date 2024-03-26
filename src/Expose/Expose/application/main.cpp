// main.cpp : This file contains the 'main' function of the expose service
//

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/network/Server.hpp"
#include "oatpp/network/ConnectionHandler.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/web/server/HttpConnectionHandler.hpp"
#include "oatpp/web/server/HttpRouter.hpp"
#include "oatpp/network/tcp/server/ConnectionProvider.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/component.hpp"


#include <iostream>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds
#include <variant>        // std::visit


#include "application/AppComponent.hpp"
#include "application/Runner.hpp"
#include "application/Expose.hpp"

void run() {
  /* Register Components in scope of run() method */
  Expose::Application::AppComponent components(
    {"localhost", 8002},      // Expose Service
    {"expose-service.virtualhost", 0}   // Expose Service Virtual Host
  );

  /* run */
  std::list<std::thread> acceptingThreads;

  Expose::Application::Runner runner;
  runner.run(acceptingThreads);

  for(auto& thread : acceptingThreads) {
    thread.join();
  }
}

int main(int argc, const char * argv[]) {

  oatpp::base::Environment::init();
  run();
  oatpp::base::Environment::destroy();
  
  return 0;
}
