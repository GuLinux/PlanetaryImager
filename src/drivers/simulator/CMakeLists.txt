file(GLOB testing_imager_SRCS *.cpp *.qrc)
add_library(testing_imager STATIC ${testing_imager_SRCS} )
add_imager_dependencies(png testing_imager )
define_driver(drivers/simulator/simulatordriver.h SimulatorDriver)
