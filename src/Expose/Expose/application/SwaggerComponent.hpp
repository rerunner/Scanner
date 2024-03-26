#pragma once

#include "application/Constants.hpp"

#include "oatpp-swagger/Model.hpp"
#include "oatpp-swagger/Resources.hpp"
#include "oatpp/core/macro/component.hpp"

namespace Expose { namespace Application {

class SwaggerComponent {
public:

  /**
   *  General API docs info
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::swagger::DocumentInfo>, swaggerDocumentInfo)(Qualifiers::SERVICE_EXPOSE, [] {

    oatpp::swagger::DocumentInfo::Builder builder;

    builder
      .setTitle("Expose Service")
      .setDescription("Monolithization Expose MS for Scanner")
      .setVersion("1.0")
      .setContactName("Oat++ Framework")
      .setContactUrl("https://oatpp.io/")

      .setLicenseName("The Unlicense")
      .setLicenseUrl("http://unlicense.org")

      .addServer("http://localhost:8002", "server on localhost");

    return builder.build();

  }());


  /**
   *  Swagger-Ui Resources (<oatpp-examples>/lib/oatpp-swagger/res)
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::swagger::Resources>, swaggerResources)(Qualifiers::SERVICE_EXPOSE, [] {
    // Correct full path to oatpp-swagger/res folder specified in cmake !!!
    return oatpp::swagger::Resources::streamResources(OATPP_SWAGGER_RES_PATH);
  }());

};

}}

