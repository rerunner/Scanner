#pragma once

#include "oatpp/web/server/api/ApiController.hpp"

#include <list>
#include <thread>

namespace Expose {
    namespace Application {
        class Runner
        {
            public:
            Runner();

            void run(std::list<std::thread>& acceptingThreads);
        };
    }
}
