// main.cpp : This file contains the 'main' function of the leveling service
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

#include "Leveling/application/AppComponent.hpp"
#include "Leveling/application/Runner.hpp"
#include "Leveling/application/Leveling.hpp"

unsigned int GSL::ACTIVE_MESSAGES = GSL::FATAL | GSL::ERROR;

void run() {
  /* Register Components in scope of run() method */
  Leveling::Application::AppComponent components(
    {"localhost", 8003},      // Leveling Service
    {"leveling-service.virtualhost", 0}   // Leveling Service Virtual Host
  );

  /* run */
  std::list<std::thread> acceptingThreads;

  Leveling::Application::Runner runner;
  runner.run(acceptingThreads);

  for(auto& thread : acceptingThreads) {
    thread.join();
  }
}

int main(int argc, const char * argv[]) 
{
  GSL::Dprintf(GSL::INFO, "Leveling application");

  oatpp::base::Environment::init();
  run();
  oatpp::base::Environment::destroy();
  
  return 0;
}
