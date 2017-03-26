set(UDEVRULES_INSTALL_DIR "/lib/udev/rules.d" CACHE STRING "Base directory for udev rules")
set(FIRMWARE_INSTALL_BASEDIR "/lib/firmware/" CACHE STRING "Base directory for firmware files")

# TODO QHY currently disabled for compilation issues
# if(NOT WIN32) # QHY currently not supported in Windows
#   option(enable_qhy "Enable QHY Driver (default: ON)" ON)
#   if(enable_qhy)
#     message("qhyccd input module enabled")
#     add_subdirectory(qhy)
#   endif(enable_qhy)
# endif()

option(enable_simulator "Enable fake driver simulator (default: OFF)" OFF) # TODO restore
if(enable_simulator)
  message("simulator input module enabled")
  add_subdirectory(simulator)
endif(enable_simulator)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  message("v4l webcam input module enabled")
  add_subdirectory(v4l2)
endif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

if(NOT WIN32) # Not supported either... but coming soon
  add_subdirectory(zwo_asi)
endif()

add_library(drivers STATIC driver.cpp imager.cpp imagercontrol.cpp imagerproperties.cpp imagerthread.cpp roi.cpp imagerexception.cpp)
add_library(supporteddrivers STATIC supporteddrivers.cpp)
add_backend_dependencies(supporteddrivers)
add_imager_dependencies(drivers)
