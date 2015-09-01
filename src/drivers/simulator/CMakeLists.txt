add_library(testing_imager STATIC simulatordriver.cpp simulator.qrc )
add_imager_dependencies(png testing_imager )
define_driver(drivers/simulator/simulatordriver.h SimulatorDriver)