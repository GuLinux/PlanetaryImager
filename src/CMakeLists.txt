set(PlanetaryImager_GUI_SRCS planetaryimager_mainwindow.cpp planetaryimager_mainwindow.ui resources/resources.qrc)
set(PlanetaryImager_SRCS planetaryimager_main.cpp ${PlanetaryImager_GUI_SRCS})


if(static_qt_windows)
add_definitions(-DSTATIC_WINDOWS_PLUGIN)
endif()

set(drivers_destination ${APPBUNDLE_INSTALL_PREFIX}lib/${CMAKE_PROJECT_NAME}/drivers)
set(binary_destination bin)
set(CMAKE_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  set(drivers_destination drivers)
  set(binary_destination .)
  add_definitions(-DDRIVERS_DIRECTORY="${drivers_destination}")
else()
  if(OSX_BUNDLE)
      add_definitions(-DDRIVERS_DIRECTORY="../../lib/PlanetaryImager/drivers")
  else()
      add_definitions(-DDRIVERS_DIRECTORY="${CMAKE_INSTALL_PREFIX}/${drivers_destination}")
  endif()
endif()


SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${drivers_destination};$ORIGIN")

if(ADD_DRIVERS_BUILD_DIRECTORY)
  add_definitions(-DADDITIONAL_DRIVERS_DIRECTORY="${CMAKE_CURRENT_BINARY_DIR}/drivers")
endif()

add_subdirectory(drivers)
add_subdirectory(image_handlers)
add_subdirectory(widgets)
add_subdirectory(commons)
add_subdirectory(network)


message(STATUS "********* ExtraLibraries: ${EXTRA_LIBRARIES}")
message("common dependencies: ${planetary_imager_commons_DEPS}")
message("backend dependencies: ${planetary_imager_backend_DEPS}")
message("frontend dependencies: ${planetary_imager_frontend_DEPS}")

set(appname planetary_imager)
set(install_targets planetary_imager)
if(WIN32 AND NOT DEVELOPER_MODE)
  set(WIN32_APP_FLAG WIN32)
  set(PlanetaryImager_SRCS ${PlanetaryImager_SRCS} ${CMAKE_SOURCE_DIR}/files/planetary_imager.rc)
endif()
if(OSX_BUNDLE)
  set(MACOSX_BUNDLE_FLAG MACOSX_BUNDLE)
  set(appname ${MACOSX_BUNDLE_BUNDLE_NAME})
endif()

add_executable(${appname} ${WIN32_APP_FLAG} ${MACOSX_BUNDLE_FLAG} ${PlanetaryImager_SRCS} planetaryimager.cpp )


target_link_libraries(${appname}
    network_server
    ${planetary_imager_backend_DEPS}
    ${planetary_imager_frontend_DEPS}
    ${planetary_imager_commons_DEPS}
    ${planetary_imager_backend_DEPS}
    ${planetary_imager_commons_DEPS}
    GuLinux_Qt_Commons
    GuLinux_c++_Commons
    ${Boost_LIBRARIES}
    ${OpenCV_LIBS}
    Qt5::Widgets
    Qt5::Qml
    ${CCFITS_LIBRARY} ${CFITSIO_LDFLAGS}
    ${Qt5_OpenGL_LIBRARIES}
    pthread
    ${EXTRA_LIBRARIES}
)

if(build_network_server)
  add_executable(planetary_imager_daemon ${WIN32_APP_FLAG} planetaryimager_daemon.cpp planetaryimager.cpp)
  target_link_libraries(planetary_imager_daemon
    network_server
    ${planetary_imager_backend_DEPS}
    ${planetary_imager_commons_DEPS}
    ${planetary_imager_backend_DEPS}
    ${planetary_imager_commons_DEPS}
    GuLinux_Qt_Commons
    GuLinux_c++_Commons
    ${Boost_LIBRARIES}
    ${OpenCV_LIBS}
    Qt5::Core
    Qt5::Network
    Qt5::Qml
    ${CCFITS_LIBRARY} ${CFITSIO_LDFLAGS}
    pthread
    ${EXTRA_LIBRARIES}
  )
  
  add_executable(planetary_imager_frontend planetaryimager_frontend.cpp planetaryimager.cpp ${PlanetaryImager_GUI_SRCS})
  target_link_libraries(planetary_imager_frontend
    network_client_gui
    network_client
    ${planetary_imager_frontend_DEPS}
    ${planetary_imager_commons_DEPS}
    GuLinux_Qt_Commons
    GuLinux_c++_Commons
    ${Boost_LIBRARIES}
    ${OpenCV_LIBS}
    Qt5::Widgets
    Qt5::Network
    Qt5::Qml
    ${CCFITS_LIBRARY} ${CFITSIO_LDFLAGS}
    ${Qt5_OpenGL_LIBRARIES}
    pthread
    ${EXTRA_LIBRARIES}
)

  set(install_targets ${install_targets} planetary_imager_daemon planetary_imager_frontend)
endif()
    
set(CMAKE_INCLUDE_POST_FILE CACHE INTERNAL "Per-environment post configuration include file")
if(CMAKE_INCLUDE_POST_FILE)
  message("Using cmake post configuration: ${CMAKE_INCLUDE_POST_FILE}")
  include(${CMAKE_INCLUDE_POST_FILE})
else()
  message("CMake post configuration file ${CMAKE_INCLUDE_POST_FILE} NOT found")
endif()


if(OSX_BUNDLE)
    install(
        TARGETS ${appname} 
        BUNDLE DESTINATION /
    )
  add_subdirectory(bundle_install)
else()
    install(TARGETS ${install_targets} RUNTIME DESTINATION ${binary_destination})
endif()


