cmake_minimum_required(VERSION 2.6)
project(qhyimager)
find_package(Qt4 REQUIRED)

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

set(QHYImager_SRCS QHYImager.cpp main.cpp)

qt4_automoc(${QHYImager_SRCS})
add_executable(qhyimager ${QHYImager_SRCS})
target_link_libraries(qhyimager ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

install(TARGETS qhyimager RUNTIME DESTINATION bin)
