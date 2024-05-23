#include <nlohmann/json.hpp>
#include "Lot.hpp"

using json = nlohmann::json;

Lot::Lot() : AggregateRootBase()
{
    state = "Loaded";

    // Create the Kafka config
    std::unique_ptr<cppkafka::Configuration> kafkaConfig;
    GSL::Dprintf(GSL::DEBUG, "Creating the Kafka config");
    std::vector<cppkafka::ConfigurationOption> kafkaConfigOptions;
    cppkafka::ConfigurationOption lotConfigOption{"metadata.broker.list", "localhost:9092"};
    kafkaConfigOptions.push_back(lotConfigOption);
    kafkaConfigOptions.push_back({ "group.id", "machinecontrol" }); // Every microservice needs its own unique kafka group id
    kafkaConfig = std::make_unique<cppkafka::Configuration>(cppkafka::Configuration{kafkaConfigOptions});

    // Create a producer instance
    GSL::Dprintf(GSL::DEBUG, "Creating a kafka producer instance");
    kafkaProducer = std::make_shared<cppkafka::Producer>(*kafkaConfig);
}

Lot::Lot(std::shared_ptr<cppkafka::Producer> newkafkaProducer) : AggregateRootBase()
{
    state = "Loaded";

    kafkaProducer = newkafkaProducer;

}

void Lot::AddWafer(Uuid wId)
{
    waferIds_.push_back(wId);
}

void Lot::RemoveWafer(Uuid wId)
{
    waferIds_.remove(wId);
}

void Lot::stateChangePublisher()
{
    json jMessage;
    jMessage.emplace("Message", "StateChange");
    jMessage.emplace("Id", id_.Get());
    jMessage.emplace("State", state);
    // serialize to CBOR
    std::vector<std::uint8_t> message = json::to_cbor(jMessage);
    cppkafka::Buffer bmess(message); // Make sure the kafka message is using the cbor binary format and not a string
    
    kafkaProducer->produce(cppkafka::MessageBuilder("lotStateTopic").partition(0).payload(bmess));
    
    try {kafkaProducer->flush();}
    catch (std::exception& e) {
        GSL::Dprintf(GSL::ERROR, "kafka flush failed with: ", e.what());
    }
}

// Boilerplate
HIBERLITE_EXPORT_CLASS(Lot)
