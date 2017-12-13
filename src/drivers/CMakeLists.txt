set(UDEVRULES_INSTALL_DIR "/lib/udev/rules.d" CACHE STRING "Base directory for udev rules")
set(FIRMWARE_INSTALL_BASEDIR "/lib/firmware/" CACHE STRING "Base directory for firmware files")


add_library(drivers STATIC driver.cpp imager.cpp imagercontrol.cpp imagerproperties.cpp imagerthread.cpp roi.cpp imagerexception.cpp)
add_library(supporteddrivers STATIC supporteddrivers.cpp)
add_backend_dependencies(supporteddrivers)
add_imager_dependencies(drivers)

add_subdirectory(zwo_asi)
add_subdirectory(simulator)
add_subdirectory(qhy)
add_subdirectory(v4l2)
add_subdirectory(iidc)
add_subdirectory(flycapture2)


message(STATUS "Will build the following drivers:")
list(REMOVE_DUPLICATES enabled_drivers)
foreach(driver ${enabled_drivers})
  message(STATUS " * ${driver}")
endforeach()  

message(STATUS "Drivers disabled but available for this platform:")
list(REMOVE_DUPLICATES disabled_drivers)
foreach(driver ${disabled_drivers})
  message(STATUS " * ${driver}")
endforeach()  


message(STATUS "Drivers unsupported for this platform:")
list(REMOVE_DUPLICATES unsupported_drivers)
foreach(driver ${unsupported_drivers})
  message(STATUS " * ${driver}")
endforeach()  
