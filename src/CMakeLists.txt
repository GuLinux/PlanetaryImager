file(GLOB PlanetaryImager_SRCS *.cpp *.ui)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

unset(planetary_imager_DEPS CACHE)
function(add_imager_dependencies)
  set(planetary_imager_DEPS ${planetary_imager_DEPS} ${ARGN} CACHE INTERNAL "planetary imager deps" FORCE)
endfunction(add_imager_dependencies)

add_subdirectory(drivers)
add_subdirectory(image_handlers)
add_subdirectory(widgets)
add_subdirectory(commons)
add_executable(planetary_imager ${PlanetaryImager_SRCS} resources/resources.qrc)

message("dependencies: ${planetary_imager_DEPS}")
  
target_link_libraries(planetary_imager
    GuLinux_Qt_Commons
    ${planetary_imager_DEPS}
    ${Boost_LIBRARIES}
    ${OpenCV_LIBS}
    ${QCustomPlot_LIBRARIES}
    Qt5::Widgets
    pthread
    ${EXTRA_LIBRARIES}
)
install(TARGETS planetary_imager RUNTIME DESTINATION bin)


