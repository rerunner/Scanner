
#include <nlohmann/json.hpp>
#include "Wafer.hpp"

using json = nlohmann::json;


Wafer::Wafer() : AggregateRootBase()
{
    state = "Loaded";

    //! Create the Kafka config
    std::vector<cppkafka::ConfigurationOption> kafkaConfigOptions;
    cppkafka::ConfigurationOption waferConfigOption{"metadata.broker.list", "localhost:9092"};
    kafkaConfigOptions.push_back(waferConfigOption);
    kafkaConfig = cppkafka::Configuration{kafkaConfigOptions};
}

Wafer::Wafer(Uuid lotId) : AggregateRootBase()
{
    parentId_ = lotId; parentLot_ = lotId; state = "Loaded";

    //! Create the Kafka config
    std::vector<cppkafka::ConfigurationOption> kafkaConfigOptions;
    cppkafka::ConfigurationOption waferConfigOption{"metadata.broker.list", "localhost:9092"};
    kafkaConfigOptions.push_back(waferConfigOption);
    kafkaConfig = cppkafka::Configuration{kafkaConfigOptions};
}

void Wafer::stateChangePublisher()
{
    json jMessage;
    if (state == "Unloaded")
    {
        GSL::Dprintf(GSL::DEBUG, "Wafer state changed to ", state, " for wafer id = ", id_.Get());
    }
    jMessage.emplace("Message", "StateChange");
    jMessage.emplace("Id", id_.Get());
    jMessage.emplace("State", state);
    // serialize to CBOR
    std::vector<std::uint8_t> message = json::to_cbor(jMessage);
    cppkafka::Buffer bmess(message); // Make sure the kafka message is using the cbor binary format and not a string
    cppkafka::Producer kafkaProducer(kafkaConfig); //! Create the Kafka producer
    kafkaProducer.produce(cppkafka::MessageBuilder("waferStateTopic").partition(0).payload(bmess));
    
    try {kafkaProducer.flush();}
    catch (std::exception& e) {
        GSL::Dprintf(GSL::ERROR, "kafka flush failed with: ", e.what());
    }
}

// Boilerplate
HIBERLITE_EXPORT_CLASS(Wafer)
