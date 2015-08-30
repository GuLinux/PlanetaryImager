include(${CMAKE_SOURCE_DIR}/GuLinux-Commons/cmake/FindMagick.cmake)

add_library(testing_imager STATIC simulatordriver.cpp )
add_imager_dependencies(png testing_imager ${MAGICK_LIBRARIES})
