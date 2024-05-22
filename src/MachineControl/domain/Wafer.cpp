
#include <nlohmann/json.hpp>
#include "Wafer.hpp"

using json = nlohmann::json;


Wafer::Wafer() : AggregateRootBase()
{
    state = "Loaded";
}

Wafer::Wafer(Uuid lotId, std::shared_ptr<cppkafka::Producer> newkafkaProducer) : AggregateRootBase()
{
    parentId_ = lotId; parentLot_ = lotId; state = "Loaded";

    kafkaProducer = newkafkaProducer;
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
    //cppkafka::Producer kafkaProducer(kafkaConfig); //! Create the Kafka producer
    kafkaProducer->produce(cppkafka::MessageBuilder("waferStateTopic").partition(0).payload(bmess));
    
    try {kafkaProducer->flush();}
    catch (std::exception& e) {
        GSL::Dprintf(GSL::ERROR, "kafka flush failed with: ", e.what());
    }
}

// Boilerplate
HIBERLITE_EXPORT_CLASS(Wafer)
