cmake_minimum_required (VERSION 3.5)

project(Scanner VERSION 1.0
	DESCRIPTION "Lithography Scanner project"
	LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
		"Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel."
	FORCE)
endif(NOT CMAKE_BUILD_TYPE)

option(BUILD_DOC "Build documentation" ON)

find_package(OpenDDS)
find_package(oatpp)
find_package(oatpp-swagger)
find_package(CURL) 
find_package(CppKafka REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)
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

include(FindPkgConfig)
pkg_check_modules(CURLPP REQUIRED curlpp)

#set(CMAKE_CXX_COMPILER ${OPENDDS_COMPILER})
set(CXXFLAGS  "-fdiagnostics-color=always -g -std=c++20 -DSTRING_NAMES=1 -DL1D_CACHE_LINE_SIZE=64")
set(CMAKE_CXX_FLAGS "${CXXFLAGS}")

set(src "${CMAKE_CURRENT_SOURCE_DIR}")
set(dst "${CMAKE_CURRENT_BINARY_DIR}")
set(all_targets Scanner)

#Some files need to be moved into the temporary destination folder
foreach(file Scanner.idl rtps.ini run_test.pl)
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


# Scanner target
add_executable(Scanner
	${src}/include/Uuid.cpp
	${src}/src/MachineControl/domain/Wafer.cpp
	${src}/src/MachineControl/domain/Lot.cpp
	${src}/src/MachineControl/domain/Chuck.cpp
	${src}/src/main.cpp
	${src}/src/MachineControl/application/MachineControl.cpp
)

# Scanner target
add_executable(Leveling
	${src}/include/Uuid.cpp
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
	${src}/include/Uuid.cpp
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

target_include_directories(Scanner PUBLIC "/usr/local/include" "/usr/include/hiberlite" "${src}/src" "${src}/src/Measure/Leveling" "${src}/src/MachineControl" "${src}/include")
target_link_libraries(Scanner OpenDDS::OpenDDS oatpp::oatpp oatpp::oatpp-swagger CppKafka::cppkafka nlohmann_json::nlohmann_json ${RAFT_LIB} ${RAFT_SHM} ${RAFT_RT} ${RAFT_AFFINITY} ${RAFT_DEMANGLE} ${RAFT_CMDARGS} ${BOOST_SYSTEM} ${HIBERLITE} ${CURL_LIBRARIES} ${CURLPP_LDFLAGS})

target_include_directories(Expose PUBLIC "/usr/local/include" "/usr/include/hiberlite" "${src}/src" "${src}/src/Expose" "${src}/src/Expose/Expose" "${src}/src/MachineControl" "${src}/src/Measure/Leveling" "${src}/include")
target_link_libraries(Expose OpenDDS::OpenDDS oatpp::oatpp oatpp::oatpp-swagger CppKafka::cppkafka nlohmann_json::nlohmann_json ${RAFT_LIB} ${RAFT_SHM} ${RAFT_RT} ${RAFT_AFFINITY} ${RAFT_DEMANGLE} ${RAFT_CMDARGS} ${BOOST_SYSTEM} ${HIBERLITE})

target_include_directories(Leveling PUBLIC "/usr/local/include" "/usr/include/hiberlite" "${src}/src" "${src}/src/Measure" "${src}/src/Measure/Leveling" "${src}/src/MachineControl" "${src}/src/Expose/Expose" "${src}/include")
target_link_libraries(Leveling OpenDDS::OpenDDS oatpp::oatpp oatpp::oatpp-swagger CppKafka::cppkafka nlohmann_json::nlohmann_json ${RAFT_LIB} ${RAFT_SHM} ${RAFT_RT} ${RAFT_AFFINITY} ${RAFT_DEMANGLE} ${RAFT_CMDARGS} ${BOOST_SYSTEM} ${HIBERLITE})
