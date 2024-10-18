// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds
#include <variant>        // std::visit
#include <xtd/xtd>

#include "MachineControl/application/MachineControl.hpp"
#include "GenLogger.hpp"

unsigned int GSL::ACTIVE_MESSAGES = GSL::FATAL | GSL::ERROR | GSL::WARNING | GSL::INFO;
RepositoryType RepositoryTypeBase::REPOSITORY_TYPE = RepositoryType::HMM;

int main(int argc, char** argv)
{
    int nrOfLots, nrOfWafersInLot;
    char *pLot, *pWafers;

    auto main_form = xtd::forms::form::create("Scanner Application");
    xtd::forms::button button1;
    xtd::forms::label label1, label2;
    xtd::forms::text_box textbox1, textbox2;
    
    button1.location({10, 80});
    button1.auto_size(true);
    button1.text("Start Scanning");
    button1.width(85);
    button1.parent(main_form);
    label1.parent(main_form);
    label1.auto_size(false);
    label1.location({10, 12});
    label1.anchor(xtd::forms::anchor_styles::top | xtd::forms::anchor_styles::right);
    label1.text("Nr. of Lots");
    textbox1.location({80, 10});
    textbox1.text("3");
    textbox1.parent(main_form);
    label2.parent(main_form);
    label2.auto_size(false);
    label2.location({10, 42});
    label2.anchor(xtd::forms::anchor_styles::top | xtd::forms::anchor_styles::right);
    label2.text("Wafers / Lot");
    textbox2.location({80, 40});
    textbox2.text("25");
    textbox2.parent(main_form);

    button1.click += [&] {
        errno = 0;
        long convLots = strtol(textbox1.text().c_str(), &pLot, 10); //Lots 
        // Check for errors: e.g., the string does not represent an integer or the integer is larger than int
        if (errno != 0 || *pLot != '\0' || convLots > INT_MAX || convLots < INT_MIN) 
        {
            // Put an error message
            std::cout << "Malformed number of Lots\n";
            nrOfLots = 1;
        } else {
            // No error
            nrOfLots = convLots;
        }
        errno = 0;
        long convWafers = strtol(textbox2.text().c_str(), &pWafers, 10); // Wafers/Lot
        if (errno != 0 || *pWafers != '\0' || convWafers > INT_MAX || convWafers < INT_MIN) 
        {
            // Put an error message
            std::cout << "Malformed number of Wafers\n";
            nrOfWafersInLot = 25;
        } else {
            // No error
            nrOfWafersInLot = convWafers;
        }
        
        GSL::Dprintf(GSL::INFO, "Scanner application running for ", nrOfLots, " Lots, and ", nrOfWafersInLot, " Wafers per Lot");
        MachineControl::MachineControl machineCTRL; // Create machine control
        machineCTRL.Initialize(); // Initialize
        machineCTRL.Execute(nrOfLots, nrOfWafersInLot); // Start Execution
    };
       
    xtd::forms::application::run(main_form);
    
#if 0
    GSL::Dprintf(GSL::INFO, "Scanner application running for ", nrOfLots, " Lots, and ", nrOfWafersInLot, " Wafers per Lot");

    MachineControl::MachineControl machineCTRL; // Create machine control
    machineCTRL.Initialize(); // Initialize
    machineCTRL.Execute(nrOfLots, nrOfWafersInLot); // Start Execution
#endif

#if 1
    //Sleep forever
    for (;;) 
    {
      std::this_thread::sleep_for (std::chrono::seconds(1));
    }
#endif

    return 0;
}
