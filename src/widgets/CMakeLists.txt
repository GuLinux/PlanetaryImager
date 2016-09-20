file(GLOB_RECURSE planetaryimager-widgets-SRCS *.cpp *.ui)
add_library(planetaryimager-widgets STATIC ${planetaryimager-widgets-SRCS})
add_imager_dependencies(planetaryimager-widgets)
