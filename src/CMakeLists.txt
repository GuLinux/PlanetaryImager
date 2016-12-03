set(PlanetaryImager_GUI_SRCS planetaryimager_mainwindow.cpp planetaryimager_mainwindow.ui resources/resources.qrc)
set(PlanetaryImager_SRCS planetaryimager.cpp ${PlanetaryImager_GUI_SRCS})
unset(planetary_imager_DEPS CACHE)
function(add_imager_dependencies)
  set(planetary_imager_DEPS ${planetary_imager_DEPS} ${ARGN} CACHE INTERNAL "planetary imager deps" FORCE)
endfunction(add_imager_dependencies)

add_subdirectory(drivers)
add_subdirectory(image_handlers)
add_subdirectory(widgets)
add_subdirectory(commons)


message("dependencies: ${planetary_imager_DEPS}")

set(install_targets planetary_imager)

add_executable(planetary_imager ${PlanetaryImager_SRCS} )
target_link_libraries(planetary_imager
    ${planetary_imager_DEPS}
    GuLinux_Qt_Commons
    GuLinux_c++_Commons
    ${Boost_LIBRARIES}
    ${OpenCV_LIBS}
    Qt5::Widgets
    CCfits ${CFITSIO_LIBRARIES} # TODO: find_package
    ${Qt5_OpenGL_LIBRARIES}
    pthread
    ${EXTRA_LIBRARIES}
)

option(build_network_server "Build PlanetaryImager network daemon and frontend" Off)
if(build_network_server)
  find_package(Qt5Network REQUIRED )
  add_subdirectory(network)
  add_executable(planetary_imager_daemon planetaryimager_daemon.cpp )
  target_link_libraries(planetary_imager_daemon
    network_server
    ${planetary_imager_DEPS}
    GuLinux_Qt_Commons
    GuLinux_c++_Commons
    ${Boost_LIBRARIES}
    ${OpenCV_LIBS}
    Qt5::Core
    Qt5::Network
    CCfits ${CFITSIO_LIBRARIES} # TODO: find_package
    pthread
    ${EXTRA_LIBRARIES}
  )
  
  add_executable(planetary_imager_frontend planetaryimager_frontend.cpp ${PlanetaryImager_GUI_SRCS})
  target_link_libraries(planetary_imager_frontend
    network_client
    ${planetary_imager_DEPS}
    GuLinux_Qt_Commons
    GuLinux_c++_Commons
    ${Boost_LIBRARIES}
    ${OpenCV_LIBS}
    Qt5::Widgets
    Qt5::Network
    CCfits ${CFITSIO_LIBRARIES} # TODO: find_package
    ${Qt5_OpenGL_LIBRARIES}
    pthread
    ${EXTRA_LIBRARIES}
)

  set(install_targets ${install_targets} planetary_imager_daemon planetary_imager_frontend)
endif()
    

install(TARGETS ${install_targets} RUNTIME DESTINATION bin)


