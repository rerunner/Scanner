#include "Wafer.hpp"

void Wafer::stateChangePublisher(std::string state)
{
    //! Create the Kafka config
    std::vector<cppkafka::ConfigurationOption> kafkaConfigOptions;
    cppkafka::ConfigurationOption exposeConfigOption{"metadata.broker.list", "localhost:9092"};
    kafkaConfigOptions.push_back(exposeConfigOption);
    kafkaConfig = std::make_unique<cppkafka::Configuration>(cppkafka::Configuration{kafkaConfigOptions});
    //! Create the Kafka producer
    kafkaProducer = std::make_unique<cppkafka::Producer>(*kafkaConfig);

    //! Produce a Kafka event message for command completion
    std::stringstream smessage;
    smessage << "NewWaferState: " << state << " for " << id_.Get();
    std::string message = smessage.str();
    kafkaProducer->produce(cppkafka::MessageBuilder("waferStateTopic").partition(0).payload(message));
    
    kafkaProducer->flush(std::chrono::milliseconds(10000)); // 10s timeout
}

// Boilerplate
HIBERLITE_EXPORT_CLASS(Wafer)
