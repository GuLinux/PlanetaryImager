file(GLOB v4l2_imager_SRCS *.cpp)
add_library(v4l2_imager MODULE ${v4l2_imager_SRCS})
target_link_libraries(v4l2_imager drivers)
