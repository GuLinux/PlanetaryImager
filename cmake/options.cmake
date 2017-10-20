# Enable fPIC (Default: ON)
option(cflags_fpic "Use -fPIC for compilation " on)
if(cflags_fpic)
  message("Compiling with -fPIC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif(cflags_fpic)
if(WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mno-ms-bitfields")
endif()

# Extra options for cmake in an external file
set(CMAKE_INCLUDE_EXT_FILE CACHE INTERNAL "Per-environment include file")
if(CMAKE_INCLUDE_EXT_FILE)
  message("Using cmake environment: ${CMAKE_INCLUDE_EXT_FILE}")
  include(${CMAKE_INCLUDE_EXT_FILE})
else()
  message("CMake environment file ${CMAKE_INCLUDE_EXT_FILE} NOT found")
endif()

# Extra libraries
SET(EXTRA_LIBRARIES "" CACHE STRING "Extra libraries for linking")
message("Extra libraries for linking: ${EXTRA_LIBRARIES}")

# Unit tests
option(ENABLE_PLANETARYIMAGER_TESTING "Enable Planetary Imager unit tests" Off)

# Search for drivers in current directory
option(ADD_DRIVERS_BUILD_DIRECTORY "Search for drivers in the current build directory (developer option)" OFF)

# Developer mode: log all message to stderr
option(DEVELOPER_MODE "Force logging mode to debug" OFF)
if(DEVELOPER_MODE)
  add_definitions(-DDEVELOPER_MODE)
endif()

option(DEBUG_NETWORK_PACKETS "Debug network packets" OFF)
if(DEBUG_NETWORK_PACKETS)
  add_definitions(-DDEBUG_NETWORK_PACKETS)
endif()

# Extra executables to be built
option(build_network_server "Build PlanetaryImager network daemon and frontend" On)
