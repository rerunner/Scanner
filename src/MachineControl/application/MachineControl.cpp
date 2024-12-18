
#include <string_view> // C++17

#include "MachineControl.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace curlpp::options;
using namespace Verdi;

namespace MachineControl
{
    MachineControl::MachineControl() : quit_(false), error_(false), waferInLotNr(0)
    {
        // Create the Kafka config
        GSL::Dprintf(GSL::DEBUG, "Creating the Kafka consumer config");
        std::vector<cppkafka::ConfigurationOption> kafkaConsumerConfigOptions;
        cppkafka::ConfigurationOption machinecontrolConsumerConfigOption{"metadata.broker.list", "localhost:9092"};
        kafkaConsumerConfigOptions.push_back(machinecontrolConsumerConfigOption);
        kafkaConsumerConfigOptions.push_back({ "group.id", "machinecontrol" }); // Every microservice needs its own unique kafka group id
        kafkaConsumerConfig = std::make_unique<cppkafka::Configuration>(cppkafka::Configuration{kafkaConsumerConfigOptions});
        
        // Create a consumer instance
        GSL::Dprintf(GSL::DEBUG, "Creating a kafka consumer instance");
        kafkaConsumer = std::make_unique<cppkafka::Consumer>(*kafkaConsumerConfig);

        // Create the Kafka config
        GSL::Dprintf(GSL::DEBUG, "Creating the Kafka producer config");
        std::vector<cppkafka::ConfigurationOption> kafkaProducerConfigOptions;
        cppkafka::ConfigurationOption machinecontrolProducerConfigOption{"metadata.broker.list", "localhost:9092"};
        kafkaProducerConfigOptions.push_back(machinecontrolProducerConfigOption);
        kafkaProducerConfig = std::make_unique<cppkafka::Configuration>(cppkafka::Configuration{kafkaProducerConfigOptions});

        // Create a producer instance
        GSL::Dprintf(GSL::DEBUG, "Creating a kafka producer instance");
        kafkaProducer = std::make_shared<cppkafka::Producer>(*kafkaProducerConfig);
    }

    MachineControl::~MachineControl()
    {
        quit_ = true;   
    }

    void MachineControl::LoadWaferOnChuck(int chuckNumber)
    {
        lotWafers.emplace_back(std::make_shared<Wafer>(currentLot->GetId(), kafkaProducer)); // Create a wafer and put it in the list
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
              error_ = true;
            break;
        }

        SwapChucks(); // Check if chucks need to be swapped
    }

    void MachineControl::ProcessMeasureStation()
    {
        // Which chuck is at the measure station?
        int chuckNumber = (scannerChucks[0].GetStation() == StationEnumType::MeasureStation)? 0 : 1;

        // What is the measure station doing?
        if (measureStation.GetCurrentState() == "Processing")
        {
            if (scannerChucks[chuckNumber].GetCurrentState() == "Loaded")
            {
                if (GetWaferState(*(scannerChucks[chuckNumber].GetWaferId())) == "ApprovedForExpose")
                {
                    // Move to Idle
                    GSL::Dprintf(GSL::DEBUG, "MeasureStation is Processing and moving to Idle");
                    measureStation.Idle();
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
                measureStation.Idle();
            }
        }
        else if (measureStation.GetCurrentState() == "Idle")
        {
            if ((scannerChucks[chuckNumber].GetCurrentState() == "Loaded") && (GetWaferState(*(scannerChucks[chuckNumber].GetWaferId())) == "Loaded"))
            {
                // Move to Processing
                GSL::Dprintf(GSL::DEBUG, "MeasureStation is Idle and moving to Processing");
                measureStation.Processing();
            }
            else 
            {  
                //Nothing
            }
        }
        else { error_ = true;/* Trouble: cannot happen. */ }
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
        if (exposeStation.GetCurrentState() == "Processing")
        {
            if (GetWaferState(*(scannerChucks[chuckNumber].GetWaferId())) == "Exposed")
            {
                // Move to Idle
                GSL::Dprintf(GSL::DEBUG, "ExposeStation is moving from Processing to Idle");
                exposeStation.Idle();
            }
            else
            {
                // Do work if needed
                ProcessWaferAtExposeStation();
            }
        }
        else if (exposeStation.GetCurrentState() == "Idle")
        {
            if ((scannerChucks[chuckNumber].GetCurrentState() == "Loaded") && (GetWaferState(*(scannerChucks[chuckNumber].GetWaferId())) == "ApprovedForExpose"))
            {
                // Move to Processing
                GSL::Dprintf(GSL::DEBUG, "ExposeStation is Idle and moving to Processing");
                exposeStation.Processing();
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
        else { error_ = true;/* Trouble: cannot happen. */ }
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
                }
                else if (!record.is_eof()) {
                    // Is it an error notification, handle it.
                    // This is explicitly skipping EOF notifications as they're not actually errors,
                    // but that's how rdkafka provides them
                    GSL::Dprintf(GSL::ERROR, "Expose kafka error");
                    error_ = true;
                }
            }
        } while(!quit_);
    }

    void MachineControl::Initialize()
    {
        scannerChucks[0].SetStation(StationEnumType::MeasureStation);
        scannerChucks[1].SetStation(StationEnumType::ExposeStation);

        nrOfLots = 0;
        nrOfWafersInLot = 0;

        // Create the PetriNet Engine
        pn = std::make_unique<ptne::PTN_Engine>(ptne::PTN_Engine::ACTIONS_THREAD_OPTION::JOB_QUEUE);
        pn->createPlace({.name="Idle",
                         .onEnterAction=IdleAction,
                         .input=true} );
        pn->createPlace({.name="Executing", 
                         .onEnterAction=ExecuteAction});
        pn->createPlace({.name="Error"});
        //transition_to_Idle
        pn->createTransition({ .name = "transition_to_Idle",
						  .activationArcs = { { .placeName = "Executing" } ,
                                              { .placeName = "Error" } },
						  .destinationArcs = { { .placeName = "Idle" } },
                          .additionalConditions={ lotsFinished } });
        //transition_to_Executing
        pn->createTransition({ .name = "transition_to_Executing",
						  .activationArcs = { { .placeName = "Idle" } },
						  .destinationArcs = { { .placeName = "Executing" } },
                          .additionalConditions={ wafersAvailable } });
        //transition_to_Error
        pn->createTransition({ .name = "transition_to_Error",
						  .activationArcs = { { .placeName = "Idle" }, { .placeName = "Executing" } },
						  .destinationArcs = { { .placeName = "Error" } },
                          .additionalConditions={ errorOccured } });
        pn->execute(false);

        eventListenerThread = std::thread(&MachineControl::eventListenerThreadHandler, this);
        
        bool levelingAlive = true;
        do {
            // Is leveling alive?
            levelingAlive = true;
            GSL::Dprintf(GSL::INFO, "Hello Leveling?");
            curlpp::options::Url myUrl(std::string("http://127.0.0.1:8003/measure/leveling/hello"));
            curlpp::Easy myRequest;
            myRequest.setOpt(myUrl);

            try {myRequest.perform();}
                catch (std::exception& e) {
                GSL::Dprintf(GSL::ERROR, "Hello Leveling Failed: ", e.what());
                levelingAlive = false;
                std::this_thread::sleep_for (std::chrono::seconds(5)); // Wait for leveling to initialize. 
            }
        } while (!levelingAlive);

        bool exposeAlive = true;
        do {
            // Is leveling alive?
            exposeAlive = true;
            GSL::Dprintf(GSL::INFO, "Hello Expose?");
            curlpp::options::Url myUrl(std::string("http://127.0.0.1:8002/expose/hello"));
            curlpp::Easy myRequest;
            myRequest.setOpt(myUrl);

            try {myRequest.perform();}
                catch (std::exception& e) {
                GSL::Dprintf(GSL::ERROR, "Hello Expose Failed: ", e.what());
                exposeAlive = false;
                std::this_thread::sleep_for (std::chrono::seconds(5)); // Wait for leveling to initialize. 
            }
        } while (!exposeAlive);

        pn->incrementInputPlace("Idle");
    }

    void MachineControl::Execute(int nrOfLotsRequested, int nrOfWafersInLotRequested)
    {
        GSL::Dprintf(GSL::INFO, "MachineControl::Execute() enter.");
        lotNr = 0;
        nrOfLots = nrOfLotsRequested;
        nrOfWafersInLot = nrOfWafersInLotRequested;
        GSL::Dprintf(GSL::INFO, "MachineControl::Execute() exit.");
    }
}
