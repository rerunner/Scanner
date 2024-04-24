// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds
#include <variant>        // std::visit

#include "MachineControl/application/MachineControl.hpp"
#include "GenLogger.hpp"

unsigned int GSL::ACTIVE_MESSAGES = GSL::INFO;

int main()
{
    GSL::Dprintf(GSL::INFO, "Scanner application");

    MachineControl::MachineControl machineCTRL; // Create machine control
    machineCTRL.Initialize(); // Initialize
    machineCTRL.Execute(); // Start Execution

#if 1
    //Sleep forever
    for (;;) 
    {
      std::this_thread::sleep_for (std::chrono::seconds(1));
    }
#endif

    return 0;
}
