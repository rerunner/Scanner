#include <nlohmann/json.hpp>
#include "Wafer.hpp"

using json = nlohmann::json;

void Wafer::stateChangePublisher()
{
    //! Create the Kafka config
    std::vector<cppkafka::ConfigurationOption> kafkaConfigOptions;
    cppkafka::ConfigurationOption exposeConfigOption{"metadata.broker.list", "localhost:9092"};
    kafkaConfigOptions.push_back(exposeConfigOption);
    kafkaConfig = new cppkafka::Configuration(cppkafka::Configuration{kafkaConfigOptions});
    //! Create the Kafka producer
    kafkaProducer = new cppkafka::Producer(*kafkaConfig);

    json jMessage;
    jMessage.emplace("Id", id_.Get());
    jMessage.emplace("State", state);
    // serialize to CBOR
    std::vector<std::uint8_t> message = json::to_cbor(jMessage);
    cppkafka::Buffer bmess(message); // Make sure the kafka message is using the cbor binary format and not a string
    kafkaProducer->produce(cppkafka::MessageBuilder("waferStateTopic").partition(0).payload(bmess));
    
    kafkaProducer->flush(std::chrono::milliseconds(10000)); // 10s timeout
}

// Boilerplate
HIBERLITE_EXPORT_CLASS(Wafer)
