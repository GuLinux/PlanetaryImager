include(FindPkgConfig)

# Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5Widgets REQUIRED )
find_package(Qt5Qml REQUIRED )
find_package(Qt5Network REQUIRED )
find_package(Qt5OpenGL)
if(Qt5OpenGL_FOUND AND NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  message("Using OpenGL renderer for image view")
  set(Qt5_OpenGL_LIBRARIES Qt5::OpenGL)
endif()
include_directories(${Qt5Widgets_INCLUDE_DIRS} ${Qt5PrintSupport_INCLUDE_DIRS})

# Boost
find_package(Boost REQUIRED)

# OpenCV
find_package(OpenCV REQUIRED )

# INDI
pkg_check_modules(LIBINDI libindi)
if(LIBINDI_FOUND)
    set(HAVE_LIBINDI On CACHE INTERNAL "")
else()
    set(HAVE_LIBINDI Off CACHE INTERNAL "")
endif()


include_directories(${OpenCV_INCLUDE_DIRS})


# ccfits
find_library(CCFITS_LIBRARY NAMES ccfits CCfits HINTS ${CCFITS_LIBRARY_PATH})
if(NOT CCFITS_LIBRARY)
    message(FATAL_ERROR "Unable to find CCfits library")
endif()

# Cfitsio
pkg_check_modules(CFITSIO REQUIRED cfitsio)
include_directories(${CFITSIO_INCLUDE_DIRS})
message("CFITSIO: ${CFITSIO_LDFLAGS}")

# LibUSB
if(NOT WIN32)
  pkg_check_modules(LIBUSB libusb-1.0 REQUIRED)
  link_directories(${LIBUSB_LIBRARY_DIRS})
endif()
include_directories(${LIBUSB_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})

# GuLinux-Commons
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/GuLinux-Commons/cmake)
include_directories(GuLinux-Commons GuLinux-Commons/c++ src)
