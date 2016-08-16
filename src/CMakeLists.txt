configure_file(version.h.in version.h)

set(QHYImager_SRCS
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
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

unset(planetary_imager_DEPS CACHE)
function(add_imager_dependencies)
  set(planetary_imager_DEPS ${planetary_imager_DEPS} ${ARGN} CACHE INTERNAL "planetary imager deps" FORCE)
endfunction(add_imager_dependencies)

add_subdirectory(drivers)
add_subdirectory(output_writers)
add_subdirectory(widgets)
add_executable(planetary_imager ${QHYImager_SRCS})



message("dependencies: ${planetary_imager_DEPS}")
if(NOT opencv_LINK_BUG)
  set(cv_libs ${OpenCV_LIBS})
endif(NOT opencv_LINK_BUG)
  
target_link_libraries(planetary_imager
    GuLinux_Qt_Commons
    drivers
    ${Boost_LIBRARIES}
    ${planetary_imager_DEPS}
    ${cv_libs}
    ${QCustomPlot_LIBRARIES}
    Qt5::Widgets
    planetaryimager-widgets
    drivers # needs to be added twice for cyclic dependency
    pthread
)
install(TARGETS planetary_imager RUNTIME DESTINATION bin)


