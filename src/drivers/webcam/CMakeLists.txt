add_library(webcam_imager STATIC webcamdriver.cpp webcamimager.cpp webcamimager_rules.cpp )
add_imager_dependencies(webcam_imager)
define_driver(drivers/webcam/webcamdriver.h WebcamDriver)