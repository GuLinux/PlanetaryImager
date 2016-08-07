unset(available_drivers_INCLUDES CACHE)
unset(available_drivers_CLASSES CACHE)
function(define_driver include class)
  set(available_drivers_INCLUDES "${available_drivers_INCLUDES}\n#include \"${include}\"" CACHE INTERNAL "driver includes" FORCE)
  set(available_drivers_CLASSES "${available_drivers_CLASSES}std::make_shared<${class}>()," CACHE INTERNAL "driver classes" FORCE)
endfunction(define_driver)

option(enable_qhy "Enable QHY Driver (default: ON)" ON)
if(enable_qhy)
  message("qhyccd input module enabled")
  add_subdirectory(qhy)
endif(enable_qhy)
#add_subdirectory(qmultimedia)
option(enable_simulator "Enable fake driver simulator (default: OFF)" OFF) # TODO restore
if(enable_simulator)
  message("simulator input module enabled")
  add_subdirectory(simulator)
endif(enable_simulator)


option(enable_opencv_webcam "Enable OpenCV webcam driver (very limited support, default: OFF)" OFF)
if(enable_opencv_webcam AND NOT opencv_LINK_BUG)
  message("v4l-opencv webcam input module enabled")
  add_subdirectory(webcam)
endif(enable_opencv_webcam AND NOT opencv_LINK_BUG)

#new v4l2 driver
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  message("v4l webcam input module enabled")
  add_subdirectory(v4l2)
endif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

add_subdirectory(zwo_asi)
configure_file(available_drivers.h.in available_drivers.h)
