cmake_minimum_required(VERSION 3.1)
enable_testing()
project(PlanetaryImager)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(MAJOR_VERSION 0)
set(MINOR_VERSION 7)
set(PATCH_VERSION 70)
set(FULL_VERSION ${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}${PACKAGE_VERSION_SUFFIX})

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(cmake/find_dependencies.cmake)
include(cmake/options.cmake)
include(cmake/utils.cmake)
include(cmake/functions.cmake)

external_project_download(cmake/GuLinux-Commons.cmake.in GuLinux-Commons)

configure_file(${CMAKE_SOURCE_DIR}/scripts/version.sh.in ${CMAKE_BINARY_DIR}/scripts/version.sh)

if(NOT OSX_BUNDLE)
    install(FILES README.md Changelog.md DESTINATION share/doc/PlanetaryImager)
endif()



add_subdirectory(${CMAKE_BINARY_DIR}/GuLinux-Commons/Qt ${CMAKE_BINARY_DIR}/GuLinux-Commons-build/Qt)
add_subdirectory(${CMAKE_BINARY_DIR}/GuLinux-Commons/c++ ${CMAKE_BINARY_DIR}/GuLinux-Commons-build/c++)
include_directories(
    ${CMAKE_BINARY_DIR}/GuLinux-Commons/Qt
    ${CMAKE_BINARY_DIR}/GuLinux-Commons/c++
    ${CMAKE_BINARY_DIR}/GuLinux-Commons
)

add_subdirectory(src)
if(ENABLE_PLANETARYIMAGER_TESTING)
  add_subdirectory(tests)
endif()

add_subdirectory(files)
add_subdirectory(support)

include(packaging.cmake)


message("Done configuring PlanetaryImager-${FULL_VERSION}-${PlanetaryImager_ARCH} for ${CMAKE_SYSTEM_NAME}")
