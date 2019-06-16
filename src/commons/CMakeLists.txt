file(GLOB planetaryimager-commons-SRCS *.cpp)
configure_file(version.h.in version.h)

add_library(planetaryimager-commons STATIC ${planetaryimager-commons-SRCS})
add_imager_dependencies(planetaryimager-commons)
