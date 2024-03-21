cmake_minimum_required (VERSION 3.5)

project(Scanner VERSION 1.0
	DESCRIPTION "Lithography Scanner project"
	LANGUAGES CXX)

find_package(OpenDDS)

find_library(RAFT_LIB raft)
find_library(RAFT_SHM shm)
find_library(RAFT_RT rt)
find_library(RAFT_AFFINITY affinity)
find_library(RAFT_DEMANGLE demangle)
find_library(RAFT_CMDARGS cmdargs)

find_library(BOOST_SYSTEM boost_system)

find_library(HIBERLITE hiberlite)

#set(CMAKE_CXX_COMPILER ${OPENDDS_COMPILER})
set(CXXFLAGS  "-fdiagnostics-color=always -g -std=c++20 -DSTRING_NAMES=1 -DL1D_CACHE_LINE_SIZE=64")
set(CMAKE_CXX_FLAGS "${CXXFLAGS}")

set(src "${CMAKE_CURRENT_SOURCE_DIR}")
set(dst "${CMAKE_CURRENT_BINARY_DIR}")
set(all_targets Scanner)

#Some files need to be moved into the temporary destination folder
foreach(file Scanner.idl rtps.ini)
	configure_file(${src}/${file} ${dst}/${file} COPYONLY)
endforeach()

# Scanner target
add_executable(Scanner
	${src}/src/main.cpp
	${src}/src/MachineControl/application/MachineControl.cpp
	${src}/src/Measure/Leveling/domain/Position.cpp
	${src}/src/Measure/Leveling/domain/Measurement.cpp
	${src}/src/Measure/Leveling/domain/WaferHeightMap.cpp
	${src}/src/Measure/Leveling/application/Leveling.cpp
  )

OPENDDS_TARGET_SOURCES(Scanner
	${dst}/Scanner.idl
  )

target_include_directories(Scanner PUBLIC "/usr/local/include" "/usr/include/hiberlite" "${src}/src" "${src}/src/Measure/Leveling" "${src}/src/MachineControl" "${src}/include")
target_link_libraries(Scanner OpenDDS::OpenDDS ${RAFT_LIB} ${RAFT_SHM} ${RAFT_RT} ${RAFT_AFFINITY} ${RAFT_DEMANGLE} ${RAFT_CMDARGS} ${BOOST_SYSTEM} ${HIBERLITE} )
