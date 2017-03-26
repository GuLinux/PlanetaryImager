file(GLOB v4l2_imager_SRCS *.cpp)
add_driver(NAME v4l2 OS Linux DEFAULT_ON SRCS ${v4l2_imager_SRCS})
