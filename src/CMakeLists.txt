set(PlanetaryImager_GUI_SRCS planetaryimager.cpp planetaryimager_mainwindow.cpp planetaryimager_mainwindow.ui resources/resources.qrc)
set(PlanetaryImager_SRCS planetaryimager.cpp ${PlanetaryImager_GUI_SRCS})
unset(planetary_imager_DEPS CACHE)
function(add_imager_dependencies)
  set(planetary_imager_DEPS ${planetary_imager_DEPS} ${ARGN} CACHE INTERNAL "planetary imager deps" FORCE)
endfunction(add_imager_dependencies)

add_subdirectory(drivers)
add_subdirectory(image_handlers)
add_subdirectory(widgets)
add_subdirectory(commons)


add_executable(planetary_imager ${PlanetaryImager_SRCS} )
add_executable(planetary_imager_daemon planetaryimager_daemon.cpp )

message("dependencies: ${planetary_imager_DEPS}")
  
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

  
target_link_libraries(planetary_imager_daemon
    ${planetary_imager_DEPS}
    GuLinux_Qt_Commons
    GuLinux_c++_Commons
    ${Boost_LIBRARIES}
    ${OpenCV_LIBS}
    Qt5::Core
    CCfits ${CFITSIO_LIBRARIES} # TODO: find_package
    pthread
    ${EXTRA_LIBRARIES}
)
install(TARGETS planetary_imager planetary_imager_daemon RUNTIME DESTINATION bin)


