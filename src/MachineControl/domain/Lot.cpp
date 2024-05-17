#include <nlohmann/json.hpp>
#include "Lot.hpp"

using json = nlohmann::json;

Lot::Lot() : AggregateRootBase()
{
    state = "Loaded"; kafkaConfig = nullptr; kafkaProducer = nullptr;

    //! Create the Kafka config
    std::vector<cppkafka::ConfigurationOption> kafkaConfigOptions;
    cppkafka::ConfigurationOption exposeConfigOption{"metadata.broker.list", "localhost:9092"};
    kafkaConfigOptions.push_back(exposeConfigOption);
    kafkaConfig = new cppkafka::Configuration(cppkafka::Configuration{kafkaConfigOptions});
    //! Create the Kafka producer
    kafkaProducer = new cppkafka::Producer(*kafkaConfig);
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
    if (state == "Unloaded")
    {
        GSL::Dprintf(GSL::DEBUG, "Lot state changed to ", state, " for lot id = ", id_.Get());
    }
    jMessage.emplace("Message", "StateChange");
    jMessage.emplace("Id", id_.Get());
    jMessage.emplace("State", state);
    // serialize to CBOR
    std::vector<std::uint8_t> message = json::to_cbor(jMessage);
    cppkafka::Buffer bmess(message); // Make sure the kafka message is using the cbor binary format and not a string
    kafkaProducer->produce(cppkafka::MessageBuilder("waferStateTopic").partition(0).payload(bmess));
    
    kafkaProducer->flush(std::chrono::milliseconds(30000)); // 30s timeout
}

// Boilerplate
HIBERLITE_EXPORT_CLASS(Lot)
