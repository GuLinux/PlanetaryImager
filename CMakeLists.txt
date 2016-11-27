cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_STANDARD 14)

enable_testing()
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/GuLinux-Commons/cmake)
include_directories(GuLinux-Commons GuLinux-Commons/c++ src)
project(PlanetaryImager)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5Widgets REQUIRED )
find_package(Qt5OpenGL)
if(Qt5OpenGL_FOUND)
  message("Using OpenGL renderer for image view")
  add_definitions(-DHAVE_QT5_OPENGL)
  set(Qt5_OpenGL_LIBRARIES Qt5::OpenGL)
endif()

include_directories(${Qt5Widgets_INCLUDE_DIRS} ${Qt5PrintSupport_INCLUDE_DIRS})

add_definitions(-DSRC_DIR="${CMAKE_SOURCE_DIR}")

find_package(Boost REQUIRED)
find_package(OpenCV REQUIRED )
#find_package(QCustomPlot REQUIRED)


include_directories(${OpenCV_INCLUDE_DIRS})
include(FindPkgConfig)

pkg_check_modules(CFITSIO REQUIRED cfitsio)

include_directories(${CFITSIO_INCLUDEDIR})

if(NOT WIN32)
  pkg_check_modules(LIBUSB libusb-1.0 REQUIRED)
  link_directories(${LIBUSB_LIBRARY_DIRS})
endif()
include_directories(${LIBUSB_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})

option(cflags_fpic "Use -fPIC for compilation " on)
if(cflags_fpic)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif(cflags_fpic)

set(MAJOR_VERSION 0)
set(MINOR_VERSION 6)
set(PATCH_VERSION 90)
set(FULL_VERSION ${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}${PACKAGE_VERSION_SUFFIX})

# adapted from QHYCCD CMakeLists.txt
 IF (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "armv6l")
  set(PlanetaryImager_ARCH armv6)
 elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "armv7")
  set(PlanetaryImager_ARCH armv7)
 elseif(CMAKE_SIZEOF_VOID_P EQUAL "8")
  set(PlanetaryImager_ARCH x86_64)
 else()
  set(PlanetaryImager_ARCH i686)
 endif (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "armv6l")

 add_definitions(-DHOST_PROCESSOR="${CMAKE_HOST_SYSTEM_PROCESSOR}" -DARCHITECTURE="${PlanetaryImager_ARCH}")
 message("Detected architecture: ${PlanetaryImager_ARCH}")
SET(EXTRA_LIBRARIES "" CACHE STRING "Extra libraries for linking")

set(PACKAGE_VERSION_SUFFIX "" CACHE STRING "Suffix package version (default: empty)")
set(PACKAGE_NAME_SUFFIX "" CACHE STRING "Suffix package name (default: empty)")

install(FILES README.md Changelog.md DESTINATION share/doc/PlanetaryImager)

INCLUDE(InstallRequiredSystemLibraries)
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "GuLinux.net Planetary Imager for Linux")
SET(CPACK_PACKAGE_VENDOR "GuLinux <marco@gulinux.net>")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
#SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/src/")
SET(CPACK_PACKAGE_VERSION_MAJOR "${MAJOR_VERSION}")
SET(CPACK_PACKAGE_VERSION_MINOR "${MINOR_VERSION}")
SET(CPACK_PACKAGE_VERSION_PATCH "${PATCH_VERSION}")
set(CPACK_PACKAGE_FILE_NAME PlanetaryImager-${FULL_VERSION}-${CMAKE_SYSTEM_NAME}-${PlanetaryImager_ARCH}${PACKAGE_NAME_SUFFIX})
SET(CPACK_PACKAGE_CONTACT "marco@gulinux.net")
#SET(CPACK_STRIP_FILES "bin/planetary_imager")

set(CPACK_SET_DESTDIR On)

SET(CPACK_GENERATOR TGZ CACHE STRING "CPack generator type (default: TGZ)")

set(CMAKE_INCLUDE_EXT_FILE CACHE INTERNAL "Per-environment include file")
if(CMAKE_INCLUDE_EXT_FILE)
  message("Using cmake environment: ${CMAKE_INCLUDE_EXT_FILE}")
  include(${CMAKE_INCLUDE_EXT_FILE})
endif()

if( CPACK_GENERATOR STREQUAL "TGZ" OR CPACK_GENERATOR STREQUAL "TBZ2")
  install(PROGRAMS support/install.sh DESTINATION "/")
endif( CPACK_GENERATOR STREQUAL "TGZ" OR CPACK_GENERATOR STREQUAL "TBZ2")

add_subdirectory(GuLinux-Commons/Qt)
add_subdirectory(GuLinux-Commons/c++)

add_subdirectory(src)
option(ENABLE_PLANETARYIMAGER_TESTING "Enable Planetary Imager unit tests" Off)
if(ENABLE_PLANETARYIMAGER_TESTING)
  add_subdirectory(tests)
endif()

add_subdirectory(files)
add_subdirectory(support)

INCLUDE(CPack)

message("Done configuring PlanetaryImager-${FULL_VERSION}-${PlanetaryImager_ARCH}")
