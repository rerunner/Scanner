
#include "MachineControl.hpp"
//#include "Measure/Leveling/application/LevelingCommand.hpp"
//#include "Expose/Expose/application/Expose.hpp"
#include "domain/Wafer.hpp"

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

using namespace curlpp::options;

namespace MachineControl
{
    void MachineControl::eventListenerThreadHandler()
    {
        // Create the Kafka config
        std::cout << "Create the Kafka config" << std::endl;
        std::vector<cppkafka::ConfigurationOption> kafkaConfigOptions;
        cppkafka::ConfigurationOption machinecontrolConfigOption{"metadata.broker.list", "localhost:9092"};
        kafkaConfigOptions.push_back(machinecontrolConfigOption);
        kafkaConfigOptions.push_back({ "group.id", "foo" });
        kafkaConfig = std::make_unique<cppkafka::Configuration>(cppkafka::Configuration{kafkaConfigOptions});
        
        // Create a consumer instance
        std::cout << "Create a consumer instance" << std::endl;
        kafkaConsumer = std::make_unique<cppkafka::Consumer>(*kafkaConfig);

        // Subscribe to topics
        std::vector<std::string> machineControlTopics;
        machineControlTopics.push_back("levelingTopic");
        std::cout << "Subscribe to topics" << std::endl;
        kafkaConsumer->subscribe(machineControlTopics);
        std::cout << "MachineControl now polling for messages from Kafka brokers" << std::endl;    
        do
        {
            // Poll messages from Kafka brokers
            cppkafka::Message record = kafkaConsumer->poll(std::chrono::milliseconds(100));
            if (record)
            {
                messageReceived = true;
                if (!record.get_error())
                {
                    std::cout << "Got a new message..." << std::endl;
                    std::cout << "    Payload [" << record.get_payload() << "]" << std::endl;
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
        quit_ = false;
        messageReceived = false;
        std::this_thread::sleep_for (std::chrono::seconds(10)); // Wait for leveling and expose to initialize. this needs to become an event
        machineControlStateMachine.on_state_transition(transition_to_Idle{});
        eventListenerThread = std::thread(&MachineControl::eventListenerThreadHandler, this);
    }

    void MachineControl::Execute()
    {
        machineControlStateMachine.on_state_transition(transition_to_Executing{});

        Wafer newWafer; // load new wafer
        newWafer.PreAligned(); // Move wafer to prealigned state (will be event later)

        // Leveling part
        {
            // Do the command to leveling
            std::cout << "starting leveling measure heightmap command with curl" << std::endl;
            curlpp::Cleanup myCleanup; // RAII cleanup
            curlpp::Easy levelingRequest;
            std::ostringstream urlCommand;
            urlCommand << "http://127.0.0.1:8003//measure/leveling/measure/" << newWafer.GetId();
            levelingRequest.setOpt(curlpp::Options::Url(std::string(urlCommand.str())));
            levelingRequest.setOpt(curlpp::Options::CustomRequest("PUT"));
            levelingRequest.perform();

            // Wait for a kafka message notifying command completion.
            while (!messageReceived) {
                    std::this_thread::sleep_for (std::chrono::seconds(1));
                }

            std::cout << std::endl << "MachineControl::Execute() -> finished leveling measure heightmap command." << std::endl;
            newWafer.Measured();
        }

        newWafer.ApprovedForExpose();

        // Expose part
        {
            std::cout << "starting expose command with curl" << std::endl;
            curlpp::Cleanup myCleanup; // RAII cleanup
            curlpp::Easy exposeRequest;
            std::ostringstream urlCommand;
            urlCommand << "http://127.0.0.1:8002/expose/expose/" << newWafer.GetId();
            exposeRequest.setOpt(curlpp::Options::Url(std::string(urlCommand.str())));
            exposeRequest.setOpt(curlpp::Options::CustomRequest("PUT"));
            exposeRequest.perform();
            std::cout << std::endl << "MachineControl::Execute() -> finished expose command." << std::endl;
            newWafer.Exposed();
        }

        newWafer.Unloaded();
        machineControlStateMachine.on_state_transition(transition_to_Idle{});
    }
}
