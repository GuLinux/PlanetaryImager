add_library(webcam_imager STATIC webcamdriver.cpp )
add_imager_dependencies(webcam_imager)
define_driver(drivers/webcam/webcamdriver.h WebcamDriver)