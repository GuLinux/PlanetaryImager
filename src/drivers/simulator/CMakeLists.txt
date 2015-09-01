include(${CMAKE_SOURCE_DIR}/GuLinux-Commons/cmake/FindImageMagick.cmake)
include_directories(${ImageMagick_INCLUDE_DIRS})

add_library(testing_imager STATIC simulatordriver.cpp simulator.qrc )
add_imager_dependencies(png testing_imager ${ImageMagick_LIBRARIES})
define_driver(drivers/simulator/simulatordriver.h SimulatorDriver)