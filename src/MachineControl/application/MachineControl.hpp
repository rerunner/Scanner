#ifndef MACHINECONTROL_H
#define MACHINECONTROL_H

#include <iostream>
#include <source_location>
#include "domain/Chuck.hpp"
#include "domain/Lot.hpp"
#include "domain/Wafer.hpp"
#include "domain/Station.hpp"

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

#include <cppkafka/cppkafka.h>

#include "PTN_Engine/PTN_Engine.h"

#include "GenLogger.hpp"
#include "infrastructure/base/UnitOfWork.hpp"

using namespace MachineControlContext;

namespace MachineControl
{
    ////////////////////
    // Class Declaration
    ////////////////////
    class MachineControl
    {
    private:
        std::unique_ptr<ptne::PTN_Engine> pn;
        curlpp::Cleanup myCleanup; // RAII cleanup
        Verdi::unitofwork::UnitOfWorkFactory UoWFactory;
        // Kafka part
        std::unique_ptr<cppkafka::Configuration> kafkaConsumerConfig;
        std::unique_ptr<cppkafka::Configuration> kafkaProducerConfig;
        std::unique_ptr<cppkafka::Consumer> kafkaConsumer;
        std::shared_ptr<cppkafka::Producer> kafkaProducer;
        std::thread eventListenerThread;
        Station measureStation;
        Station exposeStation;
        std::shared_ptr<Lot> currentLot;
        std::list<std::shared_ptr<Wafer>> lotWafers;
        std::unique_ptr<Verdi::unitofwork::UnitOfWork> executeCommandContext;
        Chuck scannerChucks[2];
        void LoadWaferOnChuck(int chuckNumber);
        void UnloadWaferFromChuck(int chuckNumber);
        std::string GetWaferState(Verdi::Uuid wId);
        void SwapChucks();
        void ProcessChuck(int chuckNumber);
        void eventListenerThreadHandler();
        void ProcessWaferAtMeasureStation();
        void ProcessMeasureStation();
        void ProcessWaferAtExposeStation();
        void ProcessExposeStation();
        bool quit_, error_;
        int waferInLotNr;
        int lotNr, nrOfLots;
        int nrOfWafersInLot;

        // Petrinet action functions
        ptne::ActionFunction IdleAction = [&](){
            GSL::Dprintf(GSL::INFO, "IdleAction Called");
        };

        ptne::ActionFunction ExecuteAction = [&](){
            GSL::Dprintf(GSL::INFO, "ExecuteAction Called");
            executeCommandContext = UoWFactory.GetNewUnitOfWork();

            for (lotNr = 0; lotNr<nrOfLots; lotNr++) // For all lots
            {
                currentLot = std::make_shared<Lot>(kafkaProducer);
                executeCommandContext->RegisterNew(currentLot);
                do
                {
                    ProcessChuck(0);
                    ProcessChuck(1);
                    ProcessMeasureStation();
                    ProcessExposeStation();
                    std::this_thread::sleep_for (std::chrono::milliseconds(1));
                } while (waferInLotNr < nrOfWafersInLot);
                waferInLotNr = 0; // But do check what it means for the last wafer in a lot! Lots must have a seamless jump
                GSL::Dprintf(GSL::INFO, "Lot ", lotNr, " finished.");
                executeCommandContext->Commit();
            } // end for all lots
            GSL::Dprintf(GSL::INFO, "ExecuteAction Finished");
            pn->incrementInputPlace("Idle");
        };
        // Petrinet condition functions 
        ptne::ConditionFunction lotsFinished = [&]() { return (lotNr==nrOfLots); };
        ptne::ConditionFunction wafersAvailable = [&]() { return ((waferInLotNr < nrOfWafersInLot) && (lotNr!=nrOfLots)); };
        ptne::ConditionFunction errorOccured = [&]() { return error_; };
    public:
        MachineControl();
        ~MachineControl();
        void Initialize();
        void Execute(int nrOfLots, int nrOfWafersInLot);
    };
}

#endif
