set(planetaryimager-commons-INCLUDE_DIRS "" CACHE INTERNAL "")
set(planetaryimager-commons-LINK-DIRS "" CACHE INTERNAL "")
set(planetaryimager-commons-LIBS "" CACHE INTERNAL "")
set(planetaryimager-commons-DEFS "" CACHE INTERNAL "")

file(GLOB planetaryimager-commons-SRCS *.cpp)
configure_file(version.h.in version.h)

add_subdirectory(mount)

add_library(planetaryimager-commons STATIC ${planetaryimager-commons-SRCS})

add_definitions(${planetaryimager-commons-DEFS})
target_include_directories(planetaryimager-commons PRIVATE ${planetaryimager-commons-INCLUDE_DIRS})
link_directories(${planetaryimager-commons-LINK-DIRS})
target_link_libraries(planetaryimager-commons ${planetaryimager-commons-LIBS})

add_imager_dependencies(planetaryimager-commons)
