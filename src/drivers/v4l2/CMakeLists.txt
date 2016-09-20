file(GLOB v4l2_imager_SRCS *.cpp)
add_library(v4l2_imager STATIC ${v4l2_imager_SRCS})
add_imager_dependencies(v4l2_imager)
define_driver(drivers/v4l2/v4l2driver.h V4L2Driver)
