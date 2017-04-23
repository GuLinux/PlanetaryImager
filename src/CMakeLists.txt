set(PlanetaryImager_GUI_SRCS planetaryimager_mainwindow.cpp planetaryimager_mainwindow.ui resources/resources.qrc)
set(PlanetaryImager_SRCS planetaryimager.cpp ${PlanetaryImager_GUI_SRCS})


unset(planetary_imager_commons_DEPS CACHE)
unset(planetary_imager_backend_DEPS CACHE)
unset(planetary_imager_frontend_DEPS CACHE)

function(add_imager_dependencies)
  set(planetary_imager_commons_DEPS ${planetary_imager_commons_DEPS} ${ARGN} CACHE INTERNAL "planetary imager common deps" FORCE)
endfunction()

function(add_backend_dependencies)
  set(planetary_imager_backend_DEPS ${planetary_imager_backend_DEPS} ${ARGN} CACHE INTERNAL "planetary imager backend deps" FORCE)
endfunction()
function(add_frontend_dependencies)
  set(planetary_imager_frontend_DEPS ${planetary_imager_frontend_DEPS} ${ARGN} CACHE INTERNAL "planetary imager frontend deps" FORCE)
endfunction()

set(drivers_destination lib/${CMAKE_PROJECT_NAME}/drivers)
set(binary_destination bin)
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  set(drivers_destination drivers)
  set(binary_destination .)
  add_definitions(-DDRIVERS_DIRECTORY="${drivers_destination}")
else()
  add_definitions(-DDRIVERS_DIRECTORY="${CMAKE_INSTALL_PREFIX}/${drivers_destination}")
endif()

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${drivers_destination};$ORIGIN")

option(ADD_DRIVERS_BUILD_DIRECTORY "Search for drivers in the current build directory (developer option)" OFF)
if(ADD_DRIVERS_BUILD_DIRECTORY)
  add_definitions(-DADDITIONAL_DRIVERS_DIRECTORY="${CMAKE_CURRENT_BINARY_DIR}/drivers")
endif()

option(DEVELOPER_MODE "Force logging mode to debug" OFF)

if(DEVELOPER_MODE)
  add_definitions(-DDEVELOPER_MODE)
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

set(install_targets planetary_imager)
if(WIN32 AND NOT DEVELOPER_MODE)
  set(WIN32_APP_FLAG WIN32)
  set(PlanetaryImager_SRCS ${PlanetaryImager_SRCS} ${CMAKE_SOURCE_DIR}/files/planetary_imager.rc)
endif()

add_executable(planetary_imager ${WIN32_APP_FLAG} ${PlanetaryImager_SRCS} )

find_library(CCFITS_LIBRARY NAMES ccfits CCfits HINTS ${CCFITS_LIBRARY_PATH})
if(NOT CCFITS_LIBRARY)
    message(FATAL_ERROR "Unable to find CCfits library")
endif()

target_link_libraries(planetary_imager
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
    CCfits ${CFITSIO_LIBRARIES} # TODO: find_package
    ${Qt5_OpenGL_LIBRARIES}
    pthread
    ${EXTRA_LIBRARIES}
)

option(build_network_server "Build PlanetaryImager network daemon and frontend" Off)
if(build_network_server)
  add_executable(planetary_imager_daemon ${WIN32_APP_FLAG} planetaryimager_daemon.cpp )
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
  
  add_executable(planetary_imager_frontend planetaryimager_frontend.cpp ${PlanetaryImager_GUI_SRCS})
  target_link_libraries(planetary_imager_frontend
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
    

install(TARGETS ${install_targets} RUNTIME DESTINATION ${binary_destination})


