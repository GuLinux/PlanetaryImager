execute_process(COMMAND ${CMAKE_SOURCE_DIR}/scripts/qhy_errors_to_map ${CMAKE_SOURCE_DIR}/QHYCCD_Linux/qhyccderr.h OUTPUT_VARIABLE qhy_error_codes)
execute_process(COMMAND ${CMAKE_SOURCE_DIR}/scripts/qhy_devices_to_map ${CMAKE_SOURCE_DIR}/QHYCCD_Linux/qhyccdcamdef.h OUTPUT_VARIABLE qhy_devices_codes)
configure_file(version.h.in version.h)

set(QHYImager_SRCS 
  main.cpp 
  planetaryimager_mainwindow.cpp 
  planetaryimager_mainwindow.ui 
  statusbarinfowidget.cpp 
  statusbarinfowidget.ui 
  saveimages.cpp 
  resources/resources.qrc
  fps_counter.cpp
  camerasettingswidget.cpp
  configurationdialog.cpp
  configurationdialog.ui
  configuration.cpp
  displayimage.cpp
  recordingpanel.cpp
  recordingpanel.ui
  drivers/imager.cpp
  drivers/driver.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

unset(planetary_imager_DEPS CACHE)
function(add_imager_dependencies)
  set(planetary_imager_DEPS ${planetary_imager_DEPS} ${ARGN} CACHE INTERNAL "planetary imager deps" FORCE)
endfunction(add_imager_dependencies)

add_subdirectory(drivers)
add_executable(planetary_imager ${QHYImager_SRCS})



message("dependencies: ${planetary_imager_DEPS}")
target_link_libraries(planetary_imager png jpeg ${OpenCV_LIBS} GuLinux_Qt_Commons Qt5::Widgets ${Boost_LIBRARIES} ${planetary_imager_DEPS})
install(TARGETS planetary_imager RUNTIME DESTINATION bin)

