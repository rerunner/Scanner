
#include <string_view> // C++17

#include "MachineControl.hpp"
#include "domain/Lot.hpp"
#include "domain/Wafer.hpp"

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

using namespace curlpp::options;

namespace MachineControl
{
    MachineControl::MachineControl() : quit_(false), 
                                       measureMessageReceived(false),
                                       exposeMessageReceived(false)
    {
    }

    MachineControl::~MachineControl()
    {
        quit_ = true;   
    }

    void MachineControl::eventListenerThreadHandler()
    {
        // Create the Kafka config
        GSL::Dprintf(GSL::INFO, "Creating the Kafka config");
        std::vector<cppkafka::ConfigurationOption> kafkaConfigOptions;
        cppkafka::ConfigurationOption machinecontrolConfigOption{"metadata.broker.list", "localhost:9092"};
        kafkaConfigOptions.push_back(machinecontrolConfigOption);
        kafkaConfigOptions.push_back({ "group.id", "foo" });
        kafkaConfig = std::make_unique<cppkafka::Configuration>(cppkafka::Configuration{kafkaConfigOptions});
        
        // Create a consumer instance
        GSL::Dprintf(GSL::INFO, "Creating a consumer instance");
        kafkaConsumer = std::make_unique<cppkafka::Consumer>(*kafkaConfig);

        // Subscribe to topics
        std::vector<std::string> machineControlTopics;
        machineControlTopics.push_back("levelingTopic");
        machineControlTopics.push_back("exposeTopic");
        GSL::Dprintf(GSL::INFO, "Subscribing to Leveling and Expose topics");
        kafkaConsumer->subscribe(machineControlTopics);
        GSL::Dprintf(GSL::INFO, "MachineControl now polling for Leveling and Expose messages from Kafka brokers");    
        do
        {
            // Poll messages from Kafka brokers
            cppkafka::Message record = kafkaConsumer->poll(std::chrono::milliseconds(100));
            if (record)
            {
                if (!record.get_error())
                {
                    GSL::Dprintf(GSL::INFO, "Got a new message...");
                    std::ostringstream newMessageStream;
                    newMessageStream << record.get_payload();
                    std::string newMessage = newMessageStream.str();
                    GSL::Dprintf(GSL::INFO, "----> Payload [", newMessage, "]");
                    if (std::string_view(newMessage.data(), 20) == "ExposeWaferCompleted")
                    {
                        exposeMessageReceived = true;
                    }
                    else if (std::string_view(newMessage.data(), 21) == "MeasureWaferCompleted")
                    {
                        measureMessageReceived = true;
                    }
                }
                else if (!record.is_eof()) {
                    // Is it an error notification, handle it.
                    // This is explicitly skipping EOF notifications as they're not actually errors,
                    // but that's how rdkafka provides them
                }
            }
        } while(!quit_);
    }

    void MachineControl::Initialize()
    {
        std::this_thread::sleep_for (std::chrono::seconds(2)); // Wait for leveling and expose to initialize. this needs to become an event
        machineControlStateMachine.on_state_transition(transition_to_Idle{});
        eventListenerThread = std::thread(&MachineControl::eventListenerThreadHandler, this);
    }

    void MachineControl::Execute()
    {
        const int nrOfLots = 1;
        const int nrOfWafersInLot = 15;
        machineControlStateMachine.on_state_transition(transition_to_Executing{});

        for (int lotNr = 0; lotNr<nrOfLots; lotNr++) // One lot for now
        {
            Lot newLot;

            for (int waferInLotNr = 1; waferInLotNr < nrOfWafersInLot; waferInLotNr++)
            {
                Wafer newWafer; // load new wafer
                newWafer.PreAligned(); // Move wafer to prealigned state (will be event later)

                // Leveling part
                {
                    // Do the command to leveling
                    GSL::Dprintf(GSL::INFO, "starting leveling measure heightmap command #", waferInLotNr, " with curl");
                    curlpp::Cleanup myCleanup; // RAII cleanup
                    curlpp::Easy levelingRequest;
                    std::ostringstream urlCommand;
                    urlCommand << "http://127.0.0.1:8003//measure/leveling/measure/" << newWafer.GetId().Get();
                    levelingRequest.setOpt(curlpp::Options::Url(std::string(urlCommand.str())));
                    levelingRequest.setOpt(curlpp::Options::CustomRequest("PUT"));
                    levelingRequest.perform();

                    // Wait for a kafka message notifying command completion.
                    while (!measureMessageReceived) {
                            std::this_thread::sleep_for (std::chrono::milliseconds(100));
                        }
                    measureMessageReceived = false;

                    std::cout << std::endl;
                    GSL::Dprintf(GSL::INFO, "Finished leveling measure heightmap command #", waferInLotNr);
                    newWafer.Measured();
                }

                newWafer.ApprovedForExpose();

                // Expose part
                {
                    GSL::Dprintf(GSL::INFO, "starting expose command #", waferInLotNr, " with curl");
                    curlpp::Cleanup myCleanup; // RAII cleanup
                    curlpp::Easy exposeRequest;
                    std::ostringstream urlCommand;
                    urlCommand << "http://127.0.0.1:8002/expose/expose/" << newWafer.GetId().Get();
                    exposeRequest.setOpt(curlpp::Options::Url(std::string(urlCommand.str())));
                    exposeRequest.setOpt(curlpp::Options::CustomRequest("PUT"));
                    exposeRequest.perform();

                    // Wait for a kafka message notifying command completion.
                    while (!exposeMessageReceived) {
                            std::this_thread::sleep_for (std::chrono::milliseconds(100));
                        }
                    exposeMessageReceived = false;

                    std::cout << std::endl;
                    GSL::Dprintf(GSL::INFO, "MachineControl::Execute() -> finished expose command #", waferInLotNr);
                    newWafer.Exposed();
                }

                newWafer.Unloaded();
            }

        } // end for all lots
        machineControlStateMachine.on_state_transition(transition_to_Idle{});
    }
}
