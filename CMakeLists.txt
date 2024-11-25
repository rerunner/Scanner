cmake_minimum_required(VERSION 3.28)

project(Scanner VERSION 1.0
	DESCRIPTION "Lithography Scanner project"
	LANGUAGES CXX)

# Set the version of C++ for the project
set(CMAKE_CXX_STANDARD 20)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
		"Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel."
	FORCE)
endif(NOT CMAKE_BUILD_TYPE)

option(BUILD_DOC "Build documentation" ON)

execute_process (
	COMMAND bash -c "pushd ${CMAKE_SOURCE_DIR}/contextmap/;sed '2d;3d' Scanner-model.cml > tempoModel1.cml;cm generate -g generic -f ../include-gen/generatedMCBoundedContextClasses.hpp -i ./tempoModel1.cml -t ./Scanner-DSL-template.ftl;rm ./tempoModel1.cml;popd"
	COMMAND bash -c "pushd ${CMAKE_SOURCE_DIR}/contextmap/;sed '1d;3d' Scanner-model.cml > tempoModel2.cml;cm generate -g generic -f ../include-gen/generatedMeasureBoundedContextClasses.hpp -i ./tempoModel2.cml -t ./Scanner-DSL-template.ftl;rm ./tempoModel2.cml;popd"
	COMMAND bash -c "pushd ${CMAKE_SOURCE_DIR}/contextmap/;sed '1d;2d' Scanner-model.cml > tempoModel3.cml;cm generate -g generic -f ../include-gen/generatedExposeBoundedContextClasses.hpp -i ./tempoModel3.cml -t ./Scanner-DSL-template.ftl;rm ./tempoModel3.cml;popd"
)

## START RAVENRB
set (RAVENCPP_SRC "${PROJECT_SOURCE_DIR}/libs/RavenDB" CACHE INTERNAL "RAVENCPP_SRC")

#fetch RavenDB client from a repo
include(FetchContent)
FetchContent_Declare(
  ravendb_client_cpp
  GIT_REPOSITORY https://github.com/rerunner/ravendb-cpp-client.git
  GIT_TAG        master
  SOURCE_DIR ${RAVENCPP_SRC}/repository SUBBUILD_DIR ${RAVENCPP_SRC}/subbuild  BINARY_DIR ${RAVENCPP_SRC}/binary
  )

FetchContent_GetProperties(ravendb_client_cpp)
if(NOT ravendb_client_cpp_POPULATED)  
  FetchContent_Populate(ravendb_client_cpp)

  #since we want only the client, we don't want to compile tests and tryouts, only the client
  set(BUILD_TRYOUTS OFF)
  set(BUILD_TESTS OFF)
  add_subdirectory("${RAVENCPP_SRC}/repository/" "${RAVENCPP_SRC}/binary")
endif() 

list(APPEND CMAKE_INCLUDE_PATH ${RAVENCPP_SRC}/repository/Raven.CppClient)
set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} CACHE INTERNAL "CMAKE_INCLUDE_PATH")

message("Raven.CppDemo include: ${RAVENCPP_SRC}/repository/Raven.CppClient")
message("CMAKE_INCLUDE_PATH : ${CMAKE_INCLUDE_PATH}")
message("CMAKE_LIBRARY_PATH : ${CMAKE_LIBRARY_PATH}")

include_directories(${CMAKE_INCLUDE_PATH})
link_directories(${CMAKE_LIBRARY_PATH})
## END RAVENRB

## START GSL
#fetch GSL library from its repo
include(FetchContent)

FetchContent_Declare(GSL
  GIT_REPOSITORY https://github.com/rerunner/GSL.git
  GIT_TAG        main
  )

# make available
FetchContent_MakeAvailable(GSL)
## END GSL

find_package(OpenDDS)
find_package(oatpp)
find_package(oatpp-swagger)
find_package(CURL) 
find_package(CppKafka REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
find_package(xtd REQUIRED)
find_package(Doxygen)

## define path to swagger-ui res folder
add_definitions(-DOATPP_SWAGGER_RES_PATH="${OATPP_BASE_DIR}/bin/oatpp-swagger/res")

find_library(RAFT_LIB raft)
find_library(RAFT_SHM shm)
find_library(RAFT_RT rt)
find_library(RAFT_AFFINITY affinity)
find_library(RAFT_DEMANGLE demangle)
find_library(RAFT_CMDARGS cmdargs)

find_library(BOOST_SYSTEM boost_system)

find_library(HIBERLITE hiberlite)

find_library(CURL_LIBRARIES curl)


#find_library(Raven_CppClient Raven.CppClient)

include(FindPkgConfig)
pkg_check_modules(CURLPP REQUIRED curlpp)

#set(CMAKE_CXX_COMPILER ${OPENDDS_COMPILER})
set(CXXFLAGS  "-fdiagnostics-color=always -g -std=c++20 -DSTRING_NAMES=1 -DL1D_CACHE_LINE_SIZE=64")
set(CMAKE_CXX_FLAGS "${CXXFLAGS}")


set(src "${CMAKE_CURRENT_SOURCE_DIR}")
set(dst "${CMAKE_CURRENT_BINARY_DIR}")
set(all_targets Scanner)

#Some files need to be moved into the temporary destination folder
foreach(file Scanner.idl rtps.ini run_all.sh)
	configure_file(${src}/${file} ${dst}/${file} COPYONLY)
endforeach()

if (DOXYGEN_FOUND)
    # set input and output files
    set(DOXYGEN_IN ${src}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${dst}/Doxyfile)

    # request to configure the file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message("Doxygen build started")

    # note the option ALL which allows to build the docs together with the application
    add_custom_target( doc_doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )
else (DOXYGEN_FOUND)
  message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)

# Create the Verdi library
add_library(Verdi)
target_include_directories(Verdi PUBLIC "/usr/local/include" "/usr/include/hiberlite" 
        "${src}/Verdi/include"
)
target_sources(Verdi
  PUBLIC
        ${src}/Verdi/src/Uuid.cpp 
)

# Scanner target
add_executable(Scanner
	${src}/src/MachineControl/domain/Wafer.cpp
	${src}/src/MachineControl/domain/Lot.cpp
	${src}/src/MachineControl/domain/Chuck.cpp
	${src}/src/main.cpp
	${src}/src/MachineControl/application/MachineControl.cpp
)

# Scanner target
add_executable(Leveling
	${src}/src/Measure/Leveling/application/main.cpp
	${src}/src/Measure/Leveling/domain/Position.cpp
	${src}/src/Measure/Leveling/domain/MarkMeasurement.cpp
	${src}/src/Measure/Leveling/domain/WaferHeightMap.cpp
	${src}/src/Measure/Leveling/application/Leveling.cpp
	${src}/src/Measure/Leveling/application/Runner.cpp
	${src}/src/Measure/Leveling/application/LevelingCommand.hpp
	${src}/src/MachineControl/domain/Wafer.cpp
	${src}/src/MachineControl/domain/Lot.cpp
)

add_executable(Expose
	${src}/src/Expose/Expose/application/main.cpp
	${src}/src/Measure/Leveling/domain/Position.cpp
	${src}/src/Measure/Leveling/domain/MarkMeasurement.cpp
	${src}/src/Measure/Leveling/domain/WaferHeightMap.cpp
	${src}/src/Expose/Expose/application/Expose.cpp
	${src}/src/Expose/Expose/application/Runner.cpp
	${src}/src/Expose/Expose/application/ExposeCommand.hpp
	${src}/src/Expose/Expose/application/DataReaderListenerImpl.cpp
  )

OPENDDS_TARGET_SOURCES(Scanner
	${dst}/Scanner.idl
  )

OPENDDS_TARGET_SOURCES(Expose
	${dst}/Scanner.idl
  )

OPENDDS_TARGET_SOURCES(Leveling
  ${dst}/Scanner.idl
)

#/usr/lib/x86_64-linux-gnu/libxxhash.a

target_include_directories(Scanner PUBLIC "/usr/local/include" "/usr/include/hiberlite" "${RAVENCPP_SRC}/repository/External/xxHash/repository" "${src}/src" "${src}/src/Measure/Leveling" "${src}/src/MachineControl" "${src}/include" "${src}/Verdi/include" "${GSL_SOURCE_DIR}/include")
target_link_libraries(Scanner Verdi GSL Raven_CppClient_static OpenDDS::OpenDDS oatpp::oatpp oatpp::oatpp-swagger CppKafka::cppkafka nlohmann_json::nlohmann_json xxhash xtd ${RAFT_LIB} ${RAFT_SHM} ${RAFT_RT} ${RAFT_AFFINITY} ${RAFT_DEMANGLE} ${RAFT_CMDARGS} ${BOOST_SYSTEM} ${HIBERLITE} ${CURL_LIBRARIES} ${CURLPP_LDFLAGS})

target_include_directories(Expose PUBLIC "/usr/local/include" "/usr/include/hiberlite" "${RAVENCPP_SRC}/repository/External/xxHash/repository" "${src}/src" "${src}/src/Expose" "${src}/src/Expose/Expose" "${src}/src/MachineControl" "${src}/src/Measure/Leveling" "${src}/include" "${src}/Verdi/include" "${GSL_SOURCE_DIR}/include")
target_link_libraries(Expose Verdi GSL Raven_CppClient_static OpenDDS::OpenDDS oatpp::oatpp oatpp::oatpp-swagger CppKafka::cppkafka nlohmann_json::nlohmann_json xxhash ${RAFT_LIB} ${RAFT_SHM} ${RAFT_RT} ${RAFT_AFFINITY} ${RAFT_DEMANGLE} ${RAFT_CMDARGS} ${BOOST_SYSTEM} ${HIBERLITE} ${CURL_LIBRARIES})

target_include_directories(Leveling PUBLIC "/usr/local/include" "/usr/include/hiberlite" "${RAVENCPP_SRC}/repository/External/xxHash/repository" "${src}/src" "${src}/src/Measure" "${src}/src/Measure/Leveling" "${src}/src/MachineControl" "${src}/src/Expose/Expose" "${src}/include" "${src}/Verdi/include" "${GSL_SOURCE_DIR}/include")
target_link_libraries(Leveling Verdi GSL Raven_CppClient_static OpenDDS::OpenDDS oatpp::oatpp oatpp::oatpp-swagger CppKafka::cppkafka nlohmann_json::nlohmann_json xxhash ${RAFT_LIB} ${RAFT_SHM} ${RAFT_RT} ${RAFT_AFFINITY} ${RAFT_DEMANGLE} ${RAFT_CMDARGS} ${BOOST_SYSTEM} ${HIBERLITE} ${CURL_LIBRARIES})
