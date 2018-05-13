file(GLOB planetaryimager-widgets-SRCS *.cpp *.ui controls/*.cpp)
add_library(planetaryimager-widgets STATIC ${planetaryimager-widgets-SRCS})
add_frontend_dependencies(planetaryimager-widgets)
