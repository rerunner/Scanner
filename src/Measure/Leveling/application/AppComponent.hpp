#pragma once

#include "Leveling/application/Constants.hpp"
#include "Leveling/application/SwaggerComponent.hpp"

#include "oatpp/web/server/HttpRouter.hpp"
#include "oatpp/network/virtual_/server/ConnectionProvider.hpp"
#include "oatpp/network/tcp/server/ConnectionProvider.hpp"
#include "oatpp/json/ObjectMapper.hpp"
#include "oatpp/macro/component.hpp"

namespace Leveling{
    namespace Application{
        struct HostPort
        {
            oatpp::String host;
            v_uint16 port;
        };

        class AppComponent
        {
            private:
            HostPort m_hostPort;
            HostPort m_virtualHost;
            public:
            AppComponent(const HostPort& hostPort, const HostPort& virtualHost) : m_hostPort(hostPort), m_virtualHost(virtualHost){}

            // Swagger component
            SwaggerComponent swaggerComponent;

            // Create virtualhost interface
            OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::virtual_::Interface>, virtualInterface)(Qualifiers::SERVICE_LEVELING, [this] 
            {
                return oatpp::network::virtual_::Interface::obtainShared(m_virtualHost.host);
            }());

            // Create "real-port" connection provider
            OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, serverConnectionProvider)(Qualifiers::SERVICE_LEVELING, [this] 
            {
                    return oatpp::network::tcp::server::ConnectionProvider::createShared({m_hostPort.host, m_hostPort.port});
            }());

            //Create "virtualhost" connection provider
            OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, virtualConnectionProvider)(Qualifiers::SERVICE_LEVELING_VH, [] 
            {
                OATPP_COMPONENT(std::shared_ptr<oatpp::network::virtual_::Interface>, interface, Qualifiers::SERVICE_LEVELING);
                return oatpp::network::virtual_::server::ConnectionProvider::createShared(interface);
            }());

            //Create Router component
            OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, httpRouter)(Qualifiers::SERVICE_LEVELING, [] 
            {
                return oatpp::web::server::HttpRouter::createShared();
            }());

            //Create ObjectMapper component to serialize/deserialize DTOs in Controller's API
            OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, apiObjectMapper)(Qualifiers::SERVICE_LEVELING, [] 
            {
		auto mapper = std::make_shared<oatpp::json::ObjectMapper>();
		return mapper;
                //return oatpp::json::ObjectMapper::createShared();
            }());
        };
    }
}
