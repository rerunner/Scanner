// entitypp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds
#include <variant>        // std::visit

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "Measure/Leveling/application/MeasureWaferCommand.hpp"

int main()
{
    std::cout << "Scanner application\n\n";

    Leveling::Leveling leveling();
    Commands::CommandExecutor executor(leveling);

    //actual execution, can be moved to queue command processor
    Commands::Command command = MeasureWafer("1"); //Wafer number 1
    std::visit(executor, command); 

    //Sleep forever
    for (;;) 
    {
      std::this_thread::sleep_for (std::chrono::seconds(1));
    }

    return 0;
}
