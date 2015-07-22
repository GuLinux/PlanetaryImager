cmake_minimum_required(VERSION 2.6)
project(qhyimager)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt5Widgets)

add_subdirectory(QHYCCD_Linux)
include_directories(QHYCCD_Linux)
add_subdirectory(src)

