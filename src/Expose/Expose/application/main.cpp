// main.cpp : This file contains the 'main' function of the expose service
//

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/network/Server.hpp"
#include "oatpp/network/ConnectionHandler.hpp"
#include "oatpp/json/ObjectMapper.hpp"
#include "oatpp/web/server/HttpConnectionHandler.hpp"
#include "oatpp/web/server/HttpRouter.hpp"
#include "oatpp/network/tcp/server/ConnectionProvider.hpp"
#include "oatpp/macro/component.hpp"

#include <iostream>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds
#include <variant>        // std::visit

#include "Expose/application/AppComponent.hpp"
#include "Expose/application/Runner.hpp"
#include "Expose/application/Expose.hpp"

using namespace Verdi;

unsigned int GSL::ACTIVE_MESSAGES = GSL::FATAL | GSL::ERROR | GSL::WARNING | GSL::INFO;
RepositoryType RepositoryTypeBase::REPOSITORY_TYPE = RepositoryType::ORM;

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

int main(int argc, const char * argv[]) 
{
  GSL::Init();
    
  GSL::Dprintf(GSL::INFO, "Expose application");

  oatpp::Environment::init();
  run();
  oatpp::Environment::destroy();
  
  return 0;
}
