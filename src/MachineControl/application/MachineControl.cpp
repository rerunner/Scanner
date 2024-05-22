
#include <string_view> // C++17

#include "MachineControl.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace curlpp::options;

namespace MachineControl
{
    MachineControl::MachineControl() : quit_(false), waferInLotNr(0)
    {
        scannerChucks[0].SetStation(StationEnumType::MeasureStation);
        scannerChucks[1].SetStation(StationEnumType::ExposeStation);

        // hiberlite boilerplate start
        hiberlite::Database *mcDB = UoWFactory.GetDataBasePtr();
        // hiberlite boilerplate end
    }

    MachineControl::~MachineControl()
    {
        quit_ = true;   
    }

    void MachineControl::LoadWaferOnChuck(int chuckNumber)
    {
        lotWafers.emplace_back(std::make_shared<Wafer>(currentLot->GetId())); // Create a wafer and put it in the list
        std::shared_ptr<Wafer> currentWafer = lotWafers.back(); // Reference without copy
        currentLot->AddWafer(currentWafer->GetId());
        scannerChucks[chuckNumber].LoadWafer(currentWafer->GetId());
        waferInLotNr++;
        executeCommandContext->RegisterNew(currentWafer->getWafer()); // Add the object to the unit of work
    }

    void MachineControl::UnloadWaferFromChuck(int chuckNumber)
    {
        // Find wafer Id
        std::shared_ptr<Uuid> wId = scannerChucks[chuckNumber].GetWaferId();

        if (wId) // At startup the chuck can be empty returning from expose station
        {
            // Access loaded wafer 
            for (const auto& it : lotWafers)
            {
                if (*wId == it->GetId())
                {
                    GSL::Dprintf(GSL::INFO, waferInLotNr, " Wafers Processed by Lot ");
                    GSL::Dprintf(GSL::DEBUG, "Wafer ", it->GetId().Get(), " finished.");
                    it->Unloaded();
                }
            }
        }
        scannerChucks[chuckNumber].UnloadWafer(); // Present or not, it's gone
    }

    std::string MachineControl::GetWaferState(Uuid wId)
    {
        //find loaded wafer and return state
        std::string result = "";
        for (const auto& it : lotWafers)
        {
            if (wId == it->GetId())
            {
                result = it->GetCurrentState();
            }
        }
        return result;
    }

    void MachineControl::SwapChucks()
    {
        if ((scannerChucks[0].GetCurrentState() == "ReadyForSwap") && (scannerChucks[1].GetCurrentState() == "ReadyForSwap"))
        {
            // Swap
            scannerChucks[0].SwapStation();
            scannerChucks[1].SwapStation();
        }
    }

    void MachineControl::ProcessChuck(int chuckNumber)
    {
        switch (scannerChucks[chuckNumber].GetStation())
        {
            case StationEnumType::MeasureStation:
                if (scannerChucks[chuckNumber].GetCurrentState() == "Unloaded")
                {
                    GSL::Dprintf(GSL::DEBUG, "MeasureStation has Chuck at state unloaded, loading Wafer");
                    LoadWaferOnChuck(chuckNumber);
                }
                if (scannerChucks[chuckNumber].GetCurrentState() == "Loaded")
                {
                    if (GetWaferState(*(scannerChucks[chuckNumber].GetWaferId())) == "ApprovedForExpose")
                    {
                        GSL::Dprintf(GSL::DEBUG, "MeasureStation has Chuck ready to Swap");
                        // Set chuck state to ReadyForSwap
                        scannerChucks[chuckNumber].SetReadyForSwap();
                    }
                }
                else if (scannerChucks[chuckNumber].GetCurrentState() == "ReadyForUnloading")
                {
                    UnloadWaferFromChuck(chuckNumber);
                }
                else
                {
                    // Nothing
                }
            break;
            case StationEnumType::ExposeStation:
                if (scannerChucks[chuckNumber].GetCurrentState() == "Unloaded")
                {
                    GSL::Dprintf(GSL::DEBUG, "ExposeStation has Chuck at state unloaded");
                    // This is a startup state: Set chuck state to ReadyForSwap
                    scannerChucks[chuckNumber].SetReadyForSwap(); 
                }
                if (scannerChucks[chuckNumber].GetCurrentState() == "Loaded")
                {
                    if (GetWaferState(*(scannerChucks[chuckNumber].GetWaferId())) == "Exposed")
                    {
                        GSL::Dprintf(GSL::DEBUG, "ExposeStation has Chuck ready to Swap");
                        // Set chuck state to ReadyForSwap
                        scannerChucks[chuckNumber].SetReadyForSwap();
                    }
                }
                else
                {
                    //Nothing
                }
            break;
            default:
              // Error
              GSL::Dprintf(GSL::ERROR, "Unknown Station!");
            break;
        }

        SwapChucks(); // Check if chucks need to be swapped
    }

    void MachineControl::ProcessMeasureStation()
    {
        // Which chuck is at the measure station?
        int chuckNumber = (scannerChucks[0].GetStation() == StationEnumType::MeasureStation)? 0 : 1;

        // What is the measure station doing?
        if (measureStation.GetStationState() == "Processing")
        {
            if (scannerChucks[chuckNumber].GetCurrentState() == "Loaded")
            {
                if (GetWaferState(*(scannerChucks[chuckNumber].GetWaferId())) == "ApprovedForExpose")
                {
                    // Move to Idle
                    GSL::Dprintf(GSL::DEBUG, "MeasureStation is Processing and moving to Idle");
                    measureStation.ReturnToIdle();
                }
                else
                {
                    // Do work if needed
                    ProcessWaferAtMeasureStation();
                }
            }
            else if (scannerChucks[chuckNumber].GetCurrentState() == "ReadyForUnloading")
            {
                // UnloadWafer
                // Return to Idle
                measureStation.ReturnToIdle();
            }
        }
        else if (measureStation.GetStationState() == "Idle")
        {
            if ((scannerChucks[chuckNumber].GetCurrentState() == "Loaded") && (GetWaferState(*(scannerChucks[chuckNumber].GetWaferId())) == "Loaded"))
            {
                // Move to Processing
                GSL::Dprintf(GSL::DEBUG, "MeasureStation is Idle and moving to Processing");
                measureStation.ProcessWafer();
            }
            else 
            {  
                //Nothing
            }
        }
        else { /* Trouble: cannot happen. */ }
    }

    void MachineControl::ProcessWaferAtMeasureStation()
    {
        // Which chuck is at the measure station?
        int chuckNumber = (scannerChucks[0].GetStation() == StationEnumType::MeasureStation)? 0 : 1;

        if (measureStation.GetCommandCompletedState())
        {
            std::shared_ptr<Uuid> wId = scannerChucks[chuckNumber].GetWaferId();
            if (GetWaferState(*wId) == "Loaded")
            {
                // Execute prealign command
                GSL::Dprintf(GSL::DEBUG, "Measurestation has Wafer at Loaded state: Execute PreAlign command");
                // find loaded wafer 
                for (const auto& it : lotWafers)
                {
                    // do whatever you wish but don't modify the list elements
                    if (*wId == it->GetId())
                    {
                        it->PreAligned(); // Move wafer to prealigned state (will be event later)
                        // Keep the command state at command completed for now
                    }
                }
            }
            else if (GetWaferState(*wId) == "PreAligned")
            {
                //commandProcessing
                GSL::Dprintf(GSL::DEBUG, "Measurestation has Wafer at PreAligned state: Execute Measure command");
                //Execute measure command
                measureStation.DoCommand();
                // Do the command to leveling
                GSL::Dprintf(GSL::DEBUG, "starting leveling measure heightmap command #", waferInLotNr, " with curl");
                curlpp::Easy levelingRequest;
                std::ostringstream urlCommand;
                urlCommand << "http://127.0.0.1:8003//measure/leveling/measure/" << wId->Get();
                levelingRequest.setOpt(curlpp::Options::Url(std::string(urlCommand.str())));
                levelingRequest.setOpt(curlpp::Options::Timeout(0));// CURLOPT_TIMEOUT_MS, 20000L)
                levelingRequest.setOpt(curlpp::Options::CustomRequest("PUT"));
                try {levelingRequest.perform();}
                 catch (std::exception& e) {
                    measureStation.CommandHasCompleted(); //Give up and retry
		        	GSL::Dprintf(GSL::ERROR, "curlpp perform failed: ", e.what());
                    std::this_thread::sleep_for (std::chrono::seconds(10));
		        }
                // The command state is false until the event comes through kafka
            }
            else if (GetWaferState(*wId) == "Measured")
            {
                //commandProcessing
                // promote Wafer state to approved for expose
                GSL::Dprintf(GSL::DEBUG, "Measurestation has Wafer at Measured state: Execute ApprovedForExpose command");
                // find loaded wafer 
                for (const auto& it : lotWafers)
                {
                    // do whatever you wish but don't modify the list elements
                    if (*wId == it->GetId())
                    {
                        it->ApprovedForExpose();
                        // Keep the state at command completed
                    }
                }
            }
            else if (GetWaferState(*wId) == "Exposed")
            {
                // On its way to be unloaded. Done by ProcessChuck
                // Keep the state at command completed
                GSL::Dprintf(GSL::DEBUG, "MeasureStation has Wafer at Exposed state, wafer will be unloaded");
            }
        }
        else
        {
            // MeasureStation wafer command has not completed yet
            std::this_thread::sleep_for (std::chrono::milliseconds(1));
        }
    }

    void MachineControl::ProcessExposeStation()
    {
        // Which chuck is at the expose station?
        int chuckNumber = (scannerChucks[0].GetStation() == StationEnumType::ExposeStation)? 0 : 1;

        // What is the expose station doing?
        if (exposeStation.GetStationState() == "Processing")
        {
            if (GetWaferState(*(scannerChucks[chuckNumber].GetWaferId())) == "Exposed")
            {
                // Move to Idle
                GSL::Dprintf(GSL::DEBUG, "ExposeStation is moving from Processing to Idle");
                exposeStation.ReturnToIdle();
            }
            else
            {
                // Do work if needed
                ProcessWaferAtExposeStation();
            }
        }
        else if (exposeStation.GetStationState() == "Idle")
        {
            if ((scannerChucks[chuckNumber].GetCurrentState() == "Loaded") && (GetWaferState(*(scannerChucks[chuckNumber].GetWaferId())) == "ApprovedForExpose"))
            {
                // Move to Processing
                GSL::Dprintf(GSL::DEBUG, "ExposeStation is Idle and moving to Processing");
                exposeStation.ProcessWafer();
            }
            else 
            {
                if (scannerChucks[chuckNumber].GetCurrentState() == "UnLoaded")
                {
                    GSL::Dprintf(GSL::DEBUG, "ExposeStation is Idle and Unloaded");
                    // Take care of startup condition!
                }
                else
                {
                    // Do nothing
                }
            } 
        }
        else { /* Trouble: cannot happen. */ }
    }

    void MachineControl::ProcessWaferAtExposeStation()
    {
        // Which chuck is at the expose station?
        int chuckNumber = (scannerChucks[0].GetStation() == StationEnumType::ExposeStation)? 0 : 1;

        if (exposeStation.GetCommandCompletedState())
        {
            std::shared_ptr<Uuid> wId = scannerChucks[chuckNumber].GetWaferId();
            if (GetWaferState(*wId) == "ApprovedForExpose")
            {
                exposeStation.DoCommand();
                GSL::Dprintf(GSL::DEBUG, "starting expose command #", waferInLotNr - 1, " with curl");
                curlpp::Easy exposeRequest;
                std::ostringstream urlCommand;
                urlCommand << "http://127.0.0.1:8002/expose/expose/" << wId->Get();
                exposeRequest.setOpt(curlpp::Options::Url(std::string(urlCommand.str())));
                exposeRequest.setOpt(curlpp::Options::Timeout(0));// CURLOPT_TIMEOUT_MS, 20000L)
                exposeRequest.setOpt(curlpp::Options::CustomRequest("PUT"));
                try {exposeRequest.perform();}
                 catch (std::exception& e) {
                    exposeStation.CommandHasCompleted(); //Give up and retry
		        	GSL::Dprintf(GSL::ERROR, "curlpp perform failed: ", e.what());
                    std::this_thread::sleep_for (std::chrono::seconds(10));
		        }
                // The command state is false until the event comes through kafka 
            }
        }
    }

    void MachineControl::eventListenerThreadHandler()
    {
        // Create the Kafka config
        GSL::Dprintf(GSL::DEBUG, "Creating the Kafka config");
        std::vector<cppkafka::ConfigurationOption> kafkaConfigOptions;
        cppkafka::ConfigurationOption machinecontrolConfigOption{"metadata.broker.list", "localhost:9092"};
        kafkaConfigOptions.push_back(machinecontrolConfigOption);
        kafkaConfigOptions.push_back({ "group.id", "machinecontrol" }); // Every microservice needs its own unique kafka group id
        kafkaConfig = std::make_unique<cppkafka::Configuration>(cppkafka::Configuration{kafkaConfigOptions});
        
        // Create a consumer instance
        GSL::Dprintf(GSL::DEBUG, "Creating a consumer instance");
        kafkaConsumer = std::make_unique<cppkafka::Consumer>(*kafkaConfig);

        // Subscribe to topics
        std::vector<std::string> machineControlTopics;
        machineControlTopics.push_back("levelingTopic");
        machineControlTopics.push_back("exposeTopic");
        //machineControlTopics.push_back("waferStateTopic");
        GSL::Dprintf(GSL::DEBUG, "Subscribing to Leveling and Expose topics");
        kafkaConsumer->subscribe(machineControlTopics);
        GSL::Dprintf(GSL::DEBUG, "MachineControl now polling for Leveling and Expose messages from Kafka brokers");    
        do
        {
            // Poll messages from Kafka brokers
            cppkafka::Message record = kafkaConsumer->poll(std::chrono::milliseconds(1000));
            
            if (record)
            {
                if (!record.get_error())
                {
                    GSL::Dprintf(GSL::DEBUG, "Got a new message...");
                    std::ostringstream newMessageStream;
                    newMessageStream << record.get_payload();
                    std::string newMessage = newMessageStream.str();
                    std::string mTopic = record.get_topic();
                    GSL::Dprintf(GSL::DEBUG, "Topic of new message is ", mTopic);
                    if (mTopic == "exposeTopic")
                    {
                        json j_message = json::from_cbor(record.get_payload()); 
                        GSL::Dprintf(GSL::DEBUG, "Message = ", j_message["Message"], " Command = ", j_message["Command"]);
                        if ((j_message["Message"] == "CommandCompleted") && (j_message["Command"] == "ExposeWafer"))
                        {
                            auto checkMessageLambda = [&](const std::shared_ptr<Wafer>& wafer) 
                            { 
                                if (j_message["Id"] == wafer->GetId().Get())
                                {
                                    GSL::Dprintf(GSL::INFO, "ExposeWaferCompleted message received for Wafer Id = ", j_message["Id"]);
                                    wafer->Exposed();
                                    exposeStation.CommandHasCompleted(); // Temporary, to be removed
                                }
                            };
                            std::for_each(lotWafers.cbegin(),lotWafers.cend(), checkMessageLambda);
                        }
                    }
                    else if (mTopic == "levelingTopic")
                    {
                        json j_message = json::from_cbor(record.get_payload()); 
                        GSL::Dprintf(GSL::DEBUG, "Message = ", j_message["Message"], " Command = ", j_message["Command"]);
                        if ((j_message["Message"] == "CommandCompleted") && (j_message["Command"] == "MeasureWafer"))
                        {
                            auto checkMessageLambda = [&](const std::shared_ptr<Wafer>& wafer) 
                            { 
                                if (j_message["Id"] == wafer->GetId().Get())
                                {
                                    GSL::Dprintf(GSL::DEBUG, "MeasureWaferCompleted message received for Wafer Id = ", j_message["Id"]);
                                    wafer->Measured();
                                    measureStation.CommandHasCompleted(); // Temporary, to be removed
                                }
                            };
                            std::for_each(lotWafers.cbegin(),lotWafers.cend(), checkMessageLambda);
                        }
                    }
#if 0
                    else if (mTopic == "waferStateTopic")
                    {
                        json j_message = json::from_cbor(record.get_payload());
                        GSL::Dprintf(GSL::INFO, "For Wafer Id = ", j_message["Id"], " new wafer state = ", j_message["State"]);
                    } 
#endif
                }
                else if (!record.is_eof()) {
                    // Is it an error notification, handle it.
                    // This is explicitly skipping EOF notifications as they're not actually errors,
                    // but that's how rdkafka provides them
                    GSL::Dprintf(GSL::ERROR, "Expose kafka error");    
                }
            }
        } while(!quit_);
    }

    void MachineControl::Initialize()
    {
        machineControlStateMachine.on_state_transition(transition_to_Idle{});
        eventListenerThread = std::thread(&MachineControl::eventListenerThreadHandler, this);
        std::this_thread::sleep_for (std::chrono::seconds(5)); // Wait for leveling and expose to initialize. this needs to become an event
    }

    void MachineControl::Execute(int nrOfLots, int nrOfWafersInLot)
    {
        executeCommandContext = UoWFactory.GetNewUnitOfWork();
        machineControlStateMachine.on_state_transition(transition_to_Executing{});

        for (int lotNr = 0; lotNr<nrOfLots; lotNr++) // For all lots
        {
            currentLot = std::make_shared<Lot>();
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
        machineControlStateMachine.on_state_transition(transition_to_Idle{});
    }
}
