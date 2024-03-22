// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds
#include <variant>        // std::visit

#include "MachineControl/application/MachineControl.hpp"

int main()
{
    std::cout << "Scanner application\n\n";

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
