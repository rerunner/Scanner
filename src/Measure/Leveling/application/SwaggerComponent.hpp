#pragma once

#include "Leveling/application/Constants.hpp"

#include "oatpp-swagger/Model.hpp"
#include "oatpp-swagger/Resources.hpp"
#include "oatpp/macro/component.hpp"

namespace Leveling { namespace Application {

class SwaggerComponent {
public:

  /**
   *  General API docs info
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::swagger::DocumentInfo>, swaggerDocumentInfo)(Qualifiers::SERVICE_LEVELING, [] {

    oatpp::swagger::DocumentInfo::Builder builder;

    builder
      .setTitle("Leveling Service")
      .setDescription("Monolithization Leveling MS for Scanner")
      .setVersion("1.0")
      .setContactName("Oat++ Framework")
      .setContactUrl("https://oatpp.io/")

      .setLicenseName("The Unlicense")
      .setLicenseUrl("http://unlicense.org")

      .addServer("http://localhost:8003", "server on localhost");

    return builder.build();

  }());


  /**
   *  Swagger-Ui Resources (<oatpp-examples>/lib/oatpp-swagger/res)
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::swagger::Resources>, swaggerResources)(Qualifiers::SERVICE_LEVELING, [] {
    // Correct full path to oatpp-swagger/res folder specified in cmake !!!
    return oatpp::swagger::Resources::loadResources(OATPP_SWAGGER_RES_PATH, true);
  }());

};

}}

