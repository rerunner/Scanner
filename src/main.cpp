// entitypp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "Measure/Leveling/application/MeasureWaferCommand.hpp"

int main()
{
    std::cout << "Scanner application\n\n";

    Commands::MeasureWaferCommand *applicationCommand; //factorise later on

    applicationCommand = new Commands::MeasureWaferCommand("1"); // Measure Wafer number one

    //Sleep forever
    for (;;) 
    {
      std::this_thread::sleep_for (std::chrono::seconds(1));
    }

    return 0;
}
