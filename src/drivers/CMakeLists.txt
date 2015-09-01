include_directories(${Qt5Widgets_INCLUDE_DIRS} ${Qt5Multimedia_INCLUDE_DIRS})

unset(available_drivers_INCLUDES CACHE)
unset(available_drivers_CLASSES CACHE)
function(define_driver include class)
  set(available_drivers_INCLUDES "${available_drivers_INCLUDES}\n#include \"${include}\"" CACHE INTERNAL "driver includes" FORCE)
  set(available_drivers_CLASSES "${available_drivers_CLASSES}std::make_shared<${class}>()," CACHE INTERNAL "driver classes" FORCE)
endfunction(define_driver)

add_subdirectory(qhy)
#add_subdirectory(qmultimedia)
option(enable_simulator "Enable fake driver simulator (default: OFF)" OFF)
if(enable_simulator)
  add_subdirectory(simulator)
endif(enable_simulator)


configure_file(available_drivers.h.in available_drivers.h)