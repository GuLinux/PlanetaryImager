# Enable fPIC (Default: ON)
option(cflags_fpic "Use -fPIC for compilation " On)
if(cflags_fpic)
  message("Compiling with -fPIC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif(cflags_fpic)
if(WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mno-ms-bitfields")
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
option(DEBUG_NETWORK_PACKETS "Debug network packets" OFF)
option(DISABLE_TRACKING "Disable tracking implementation" ON)

# Extra executables to be built
if("${CPACK_GENERATOR}" STREQUAL "DragNDrop")
    message("Disabling PlanetaryImager network daemon and frontend in bundle mode")
    set(build_network_server Off)
    set(OSX_BUNDLE On)
    set(CMAKE_INSTALL_PREFIX /)
    set(MACOSX_BUNDLE_BUNDLE_NAME PlanetaryImager)
    set(MACOSX_BUNDLE_BUNDLE_VERSION ${FULL_VERSION})
    set(MACOSX_BUNDLE_GUI_IDENTIFIER net.gulinux.planetaryimager)
    set(APPBUNDLE_INSTALL_PREFIX "${MACOSX_BUNDLE_BUNDLE_NAME}.app/")
else()
    option(build_network_server "Build PlanetaryImager network daemon and frontend" On)
endif()
