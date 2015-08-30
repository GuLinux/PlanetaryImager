include_directories(${Qt5Widgets_INCLUDE_DIRS} ${Qt5Multimedia_INCLUDE_DIRS})

add_subdirectory(qhy)
#add_subdirectory(qmultimedia)
option(enable_simulator "Enable fake driver simulator (default: OFF)" OFF)
if(enable_simulator)
  add_subdirectory(simulator)
  add_definitions(-DHAVE_SIMULATOR=1)
endif(enable_simulator)


configure_file(available_drivers.h.in available_drivers.h)