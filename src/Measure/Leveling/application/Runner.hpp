#pragma once

#include "oatpp/web/server/api/ApiController.hpp"

#include <list>
#include <thread>

namespace Leveling {
    namespace Application {
        class Runner
        {
            public:
            Runner();

            void run(std::list<std::thread>& acceptingThreads);
        };
    }
}
