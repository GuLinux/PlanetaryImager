#TODO replace this script invokation
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(QHY_INCLUDE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/QHYCCD_Windows_SDK_x64_LatestVersion/x64/include)
  set(QHY_DLL_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/QHYCCD_Windows_SDK_x64_LatestVersion/x64/dll)
  include_directories(${QHY_INCLUDE_DIRECTORY})
else()
  set(QHY_INCLUDE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/QHYCCD_Linux)
  set(QHY_LIBS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/QHYCCD_Linux)
endif()
execute_process(COMMAND ${CMAKE_SOURCE_DIR}/scripts/qhy_errors_to_map ${QHY_INCLUDE_DIRECTORY}/qhyccderr.h OUTPUT_VARIABLE qhy_error_codes)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/qhyccdstruct.h 
  COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/qhy_fix_libusb_include.py ${QHY_INCLUDE_DIRECTORY}/qhyccdstruct.h ${CMAKE_CURRENT_BINARY_DIR}/qhyccdstruct.h
)
add_custom_target(qhyccdstruct DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/qhyccdstruct.h )

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
  configure_file(${QHY_INCLUDE_DIRECTORY}/qhyccd.h ${CMAKE_CURRENT_BINARY_DIR}/ COPYONLY)
  configure_file(${QHY_INCLUDE_DIRECTORY}/qhyccdcamdef.h ${CMAKE_CURRENT_BINARY_DIR}/ COPYONLY)
  configure_file(${QHY_INCLUDE_DIRECTORY}/qhyccderr.h ${CMAKE_CURRENT_BINARY_DIR}/ COPYONLY)
  include_directories(${CMAKE_CURRENT_BINARY_DIR})
endif()

# TODO automatically get from QHYCCD CMakeLists.txt
set(LIBQHY_VERSION "0.1.8")
set(LIBQHY_SOVERSION "0")

#TODO replace this script invokation


set(qhy_SOURCES qhyccdimager.cpp qhydriver.cpp qhyimagingworker.cpp qhyexception.cpp qhycontrol.cpp)

set(QHY_LIBRARY_armv6 ${QHY_LIBS_DIRECTORY}/libqhy_arm_v6.bin)
set(QHY_LIBRARY_armv7 ${QHY_LIBS_DIRECTORY}/libqhy_arm_v7.bin)
set(QHY_LIBRARY_x86_64 ${QHY_LIBS_DIRECTORY}/libqhy_64.bin)
set(QHY_LIBRARY_i686 ${QHY_LIBS_DIRECTORY}/libqhy_32.bin)

set(QHY_LIBRARY ${QHY_LIBRARY_${PlanetaryImager_ARCH}})

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(QHY_LIBRARY ${QHY_DLL_DIRECTORY}/qhyccd.dll)
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(QHY_LIBRARY ${QHY_LIBS_DIRECTORY}/libqhy_osx.a)
  set(QHY_EXTRA_LIBRARIES usb-1.0  "-framework CoreFoundation" "-framework IOKit" objc)
endif()


# if(ADD_DRIVERS_BUILD_DIRECTORY)
#   get_filename_component(QHY_LIBRARY_NAME ${QHY_LIBRARY} NAME)
#   file(COPY ${QHY_LIBRARY} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
#   file(RENAME ${CMAKE_CURRENT_BINARY_DIR}/${QHY_LIBRARY_NAME} ${CMAKE_CURRENT_BINARY_DIR}/libqhy.so.${LIBQHY_VERSION})
# endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  install(FILES QHYCCD_Linux/85-qhy.rules DESTINATION ${UDEVRULES_INSTALL_DIR})
  install(FILES ${QHY_LIBRARY} DESTINATION ${drivers_destination} RENAME libqhy.so.${LIBQHY_VERSION})

  file(GLOB QHY_FIRMWARE_FILES QHYCCD_Linux/firmware/*)
  install(FILES ${QHY_FIRMWARE_FILES} DESTINATION ${FIRMWARE_INSTALL_BASEDIR}/qhy)
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  install(FILES ${QHY_LIBRARY} DESTINATION .)
endif()
add_driver(NAME qhy OS Linux Windows Darwin DEFAULT_ON SRCS ${qhy_SOURCES} ${qhy_GENERATED_SOURCES} LINK ${QHY_LIBRARY} ${QHY_EXTRA_LIBRARIES})
if(BUILD_DRIVER_qhy)
  add_dependencies(qhy qhyccdstruct)
endif()
