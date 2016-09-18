configure_file(version.h.in version.h)

set(PlanetaryImager_SRCS
  main.cpp 
  planetaryimager_mainwindow.cpp 
  planetaryimager_mainwindow.ui 
  saveimages.cpp 
  resources/resources.qrc
  fps_counter.cpp
  configurationdialog.cpp
  configurationdialog.ui
  configuration.cpp
  displayimage.cpp
  histogram.cpp
  recordinginformation.cpp
  frame.cpp
  threadimagehandler.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

unset(planetary_imager_DEPS CACHE)
function(add_imager_dependencies)
  set(planetary_imager_DEPS ${planetary_imager_DEPS} ${ARGN} CACHE INTERNAL "planetary imager deps" FORCE)
endfunction(add_imager_dependencies)

add_subdirectory(drivers)
add_subdirectory(output_writers)
add_subdirectory(widgets)
add_executable(planetary_imager ${PlanetaryImager_SRCS})



message("dependencies: ${planetary_imager_DEPS}")
  
target_link_libraries(planetary_imager
    GuLinux_Qt_Commons
    drivers
    planetaryimager-widgets
    ${Boost_LIBRARIES}
    ${planetary_imager_DEPS}
    ${OpenCV_LIBS}
    ${QCustomPlot_LIBRARIES}
    Qt5::Widgets
    drivers # needs to be added twice for cyclic dependency
    pthread
    ${EXTRA_LIBRARIES}
)
install(TARGETS planetary_imager RUNTIME DESTINATION bin)


