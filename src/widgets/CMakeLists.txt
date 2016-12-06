file(GLOB_RECURSE planetaryimager-widgets-SRCS *.cpp *.ui)
add_library(planetaryimager-widgets STATIC ${planetaryimager-widgets-SRCS})
add_frontend_dependencies(planetaryimager-widgets)
