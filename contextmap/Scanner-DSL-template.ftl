#pragma once
// Auto-generated by user ${userName} at ${timestamp} from the Context Map '${contextMap.name}' stored in ${filename}
// using contextMapper version <#if contextMapperVersion?has_content>${contextMapperVersion}<#else>unknown</#if>

#include <list>
#include "domain/base/AggregateRootBase.hpp"
#include "domain/base/ValueObjectBase.hpp"
#include "hiberlite.h"
#include <nlohmann/json.hpp>
#include <cppkafka/cppkafka.h>
#include "FiniteStateMachine.hpp"

<#import "domain-object-attribute-and-operation-tables.ftl" as attrOpsMacro>

<#assign allEntityNames = [] />
<#assign allValueobjectNames = [] />
<#assign oneToManyRefs = [] />
<#assign oneToOneRefs = [] />
<#assign allJsonBoilerPlate = [] />

<#list contextMap.boundedContexts as bc>
<#list bc.aggregates as agg>
<#assign entities = [] />
<#assign entityNames = [] />
<#assign valueobjects = [] />
<#assign valueobjectNames = [] />
<#assign enums = [] />
<#assign enumNames = [] />
<#assign jsonBoilerPlate = [] />
<#assign entities = entities + agg.domainObjects?filter(dob -> instanceOf(dob, Entity))>
<#assign valueobjects = valueobjects + agg.domainObjects?filter(dob -> instanceOf(dob, ValueObject))>
<#assign enums = enums + agg.domainObjects?filter(dob -> instanceOf(dob, Enum))>
<#if entities?has_content>
// Bounded Context ${bc.name} '${bc.domainVisionStatement}'<#lt>
// is of type ${bc.type}
// and is responsible for ${bc.responsibilities?join(", ")}
namespace ${bc.name} {
<#if enums?has_content>
<#list enums as enum>
<#if enum.isDefinesAggregateLifecycle()>
    // found aggregate lifecycle enum ${enum.name}
    namespace ${agg.name?lower_case}State {
    <#list enum.getValues() as enumVal>
        struct ${enumVal.name};
    </#list>

        // specific supported state transition types
    <#list enum.getValues() as enumVal>
        struct transition_to_${enumVal.name};
    </#list>
    } // namespace ${agg.name?lower_case}State

    using ${agg.name?lower_case}_state_machine = GSL::state_machine<
<#list enum.getValues() as enumVal>
                                ${agg.name?lower_case}State::${enumVal.name}<#if enumVal?has_next>,<#else>></#if>
</#list> 
<#else>

    // found non-aggregate lifecycle enum ${enum.name}
    enum  ${enum.name}
    {
    <#list enum.getValues() as enumVal>
        ${enumVal.name}<#if enumVal?has_next>,<#else></#if>
    </#list> 
    }
</#if>
</#list>
</#if>
<#list entities as entity>

    <#if entity.aggregateRoot>
    class ${entity.name} : public Verdi::AggregateRootBase 
    {
    private:
        <#if enums?has_content>
        <#list enums as enum>
        <#if enum.isDefinesAggregateLifecycle()>
        // Aggregate lifecycle state part
        ${agg.name?lower_case}_state_machine ${agg.name?lower_case}StateMachine;
        std::string state; 
        // Kafka part
        std::shared_ptr<cppkafka::Producer> kafkaProducer;
        void stateChangePublisher()
        {   json jMessage;
            jMessage.emplace("Message", "StateChange");
            jMessage.emplace("Id", id_.Get());
            jMessage.emplace("State", state);
            // serialize to CBOR
            std::vector<std::uint8_t> message = json::to_cbor(jMessage);
            cppkafka::Buffer bmess(message); // Make sure the kafka message is using the cbor binary format and not a string
            kafkaProducer->produce(cppkafka::MessageBuilder("${agg.name?lower_case}StateTopic").partition(0).payload(bmess));
            try {kafkaProducer->flush();}
            catch (std::exception& e) 
            {   GSL::Dprintf(GSL::ERROR, "kafka flush failed with: ", e.what());
            }
        }
        </#if>
        </#list>
        </#if>
    <#else>
    class ${entity.name} : public Verdi::EntityBase 
    {
    private:
    </#if>
        // attributes
        <#list entity.attributes as attribute>
        ${attribute.type} ${attribute.name};
        </#list>
        // references
        <#list entity.references as reference>
        <#if reference.collectionType?has_content && reference.collectionType.name() == "LIST">
        std::list<${reference.domainObjectType.name}> ${reference.name}; // ${reference.collectionType.name()}
        <#assign jsonBoilerPlate = entity.references?map(e -> e.name)>
        <#else>
        ${reference.domainObjectType.name} ${reference.name}; // ${reference.collectionType.name()}
        <#assign jsonBoilerPlate = entity.references?map(e -> e.name)>
        </#if>
        <#assign allJsonBoilerPlate = jsonBoilerPlate>
        </#list>
        <#if allJsonBoilerPlate?has_content>
        // Hiberlite boilerplate start
        friend class hiberlite::access;
        template < class Archive >
        void hibernate(Archive & ar)
        {   ar & HIBERLITE_NVP(id_); // From Base class
            ar & HIBERLITE_NVP(parentId_); // From Base class
            <#list allJsonBoilerPlate as jbp>
	        ar & HIBERLITE_NVP(${jbp}_);
            </#list>
        }
        // Hiberlite boilerplate end
        </#if>
    public:
        <#if entity.aggregateRoot>
        <#if enums?has_content>
        <#list enums as enum>
        <#if enum.isDefinesAggregateLifecycle()>
        <#assign enumVal = enum.getValues()>
        ${entity.name}()
        {   state = "${enumVal[0].name}"; // First declared state is initial state
            // Create the Kafka config
            std::unique_ptr<cppkafka::Configuration> kafkaConfig;
            GSL::Dprintf(GSL::DEBUG, "Creating the Kafka config");
            std::vector<cppkafka::ConfigurationOption> kafkaConfigOptions;
            cppkafka::ConfigurationOption ${agg.name?lower_case}ConfigOption{"metadata.broker.list", "localhost:9092"};
            kafkaConfigOptions.push_back(${agg.name?lower_case}ConfigOption);
            kafkaConfig = std::make_unique<cppkafka::Configuration>(cppkafka::Configuration{kafkaConfigOptions});
            // Create a Kafka producer instance
            GSL::Dprintf(GSL::DEBUG, "Creating a kafka producer instance");
            kafkaProducer = std::make_shared<cppkafka::Producer>(*kafkaConfig);
        }
        ${entity.name}(std::shared_ptr<cppkafka::Producer> newkafkaProducer)
        {   state = "${enumVal[0].name}"; // First declared state is initial state
            kafkaProducer = newkafkaProducer;
        }
        std::string GetCurrentState() const {return state;}
        </#if>
        </#list>
        <#list enums as enum>
        <#if enum.isDefinesAggregateLifecycle()>
        // Aggregate lifecycle state changes for ${enum.name}
        <#list enum.getValues() as enumVal>
        void ${enumVal.name}()
        {   ${agg.name?lower_case}StateMachine.on_state_transition(${agg.name?lower_case}State::transition_to_${enumVal.name}{});
            state = "${enumVal.name}";
            stateChangePublisher();
        }
        </#list> 
        </#if>
        </#list>
        <#else>
        ${entity.name}(){}
        </#if>
        </#if>
        Uuid Get${entity.name}Id(void){return id_;} // Id inherited from base class
        <@attrOpsMacro.renderDomainObjectOperationsAndAttributes entity />
        virtual ~${entity.name}(){}
        // RavenDB & FFS boilerplate start
        <#if allJsonBoilerPlate?has_content>
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(${entity.name}, ${allJsonBoilerPlate?join(", ")})
	    </#if>
        // RavenDB & FFS boilerplate end
    };

</#list> 
</#if>
<#assign valueobjectNames = valueobjects?map(e -> e.name)>
<#assign allValueobjectNames = allValueobjectNames + valueobjectNames>
<#if valueobjects?has_content>
<#list valueobjects as valueobject>

    class ${valueobject.name} : public Verdi::ValueObjectBase 
    {
    private:
        <#list valueobject.attributes as attribute>
	    ${attribute.type} ${attribute.name};
        </#list>
    public:
    };
</#list> 
</#if>

} // namespace ${bc.name}

</#list> 
</#list>
<#assign entityNames = entities?map(e -> e.name)>
<#assign allEntityNames = allEntityNames + entityNames>
<#assign enumNames = enums?map(e -> e.name)>