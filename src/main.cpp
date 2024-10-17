// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds
#include <variant>        // std::visit

#include "MachineControl/application/MachineControl.hpp"
#include "GenLogger.hpp"

unsigned int GSL::ACTIVE_MESSAGES = GSL::FATAL | GSL::ERROR | GSL::WARNING | GSL::INFO;
RepositoryType RepositoryTypeBase::REPOSITORY_TYPE = RepositoryType::HMM;

int main(int argc, char** argv)
{
    int nrOfLots, nrOfWafersInLot;
    char *pLot, *pWafers;

    if (argc < 2)
    {
        std::cout << "Need number of Lots and number of Wafers per Lot \n";
        exit(-1);
    }

    errno = 0;
    long convLots = strtol(argv[1], &pLot, 10); //Lots 
    // Check for errors: e.g., the string does not represent an integer or the integer is larger than int
    if (errno != 0 || *pLot != '\0' || convLots > INT_MAX || convLots < INT_MIN) 
    {
        // Put an error message
        std::cout << "Malformed number of Lots\n";
        exit(-1);
    } else {
        // No error
        nrOfLots = convLots;
    }
    errno = 0;
    long convWafers = strtol(argv[2], &pWafers, 10); // Wafers/Lot
    if (errno != 0 || *pWafers != '\0' || convWafers > INT_MAX || convWafers < INT_MIN) 
    {
        // Put an error message
        std::cout << "Malformed number of Wafers\n";
        exit(-1);
    } else {
        // No error
        nrOfWafersInLot = convWafers;
    }
    

    GSL::Dprintf(GSL::INFO, "Scanner application running for ", nrOfLots, " Lots, and ", nrOfWafersInLot, " Wafers per Lot");

    MachineControl::MachineControl machineCTRL; // Create machine control
    machineCTRL.Initialize(); // Initialize
    machineCTRL.Execute(nrOfLots, nrOfWafersInLot); // Start Execution

#if 1
    //Sleep forever
    for (;;) 
    {
      std::this_thread::sleep_for (std::chrono::seconds(1));
    }
#endif

    return 0;
}
