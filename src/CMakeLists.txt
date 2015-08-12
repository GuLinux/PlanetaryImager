execute_process(COMMAND ${CMAKE_SOURCE_DIR}/scripts/qhy_errors_to_map ${CMAKE_SOURCE_DIR}/QHYCCD_Linux/qhyccderr.h OUTPUT_VARIABLE qhy_error_codes)
execute_process(COMMAND ${CMAKE_SOURCE_DIR}/scripts/qhy_devices_to_map ${CMAKE_SOURCE_DIR}/QHYCCD_Linux/qhyccdcamdef.h OUTPUT_VARIABLE qhy_devices_codes)


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
add_subdirectory(drivers)
add_executable(planetary_imager ${QHYImager_SRCS})

if(CMAKE_SIZEOF_VOID_P MATCHES "8")
  set(QHY_LIBRARY ${CMAKE_SOURCE_DIR}/QHYCCD_Linux/libqhy_64.bin)
else(CMAKE_SIZEOF_VOID_P MATCHES "8")
  set(QHY_LIBRARY ${CMAKE_SOURCE_DIR}/QHYCCD_Linux/libqhy_32.bin)
endif(CMAKE_SIZEOF_VOID_P MATCHES "8")


target_link_libraries(planetary_imager qhy_driver ${QHY_LIBRARY} qmultimedia_driver Qt5::Widgets Qt5::Multimedia )

install(TARGETS planetary_imager RUNTIME DESTINATION bin)

