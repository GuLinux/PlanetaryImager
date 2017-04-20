if(BUILD_DRIVER_qhy)
  #TODO replace this script invokation
  execute_process(COMMAND ${CMAKE_SOURCE_DIR}/scripts/qhy_errors_to_map ${CMAKE_CURRENT_SOURCE_DIR}/QHYCCD_Linux/qhyccderr.h OUTPUT_VARIABLE qhy_error_codes)

  configure_file(QHYCCD_Linux/qhyccd.h ${CMAKE_CURRENT_BINARY_DIR}/ COPYONLY)
  configure_file(QHYCCD_Linux/qhyccdcamdef.h ${CMAKE_CURRENT_BINARY_DIR}/ COPYONLY)
  configure_file(QHYCCD_Linux/qhyccdstruct.h ${CMAKE_CURRENT_BINARY_DIR}/ COPYONLY)
  configure_file(QHYCCD_Linux/qhyccderr.h ${CMAKE_CURRENT_BINARY_DIR}/ COPYONLY)

  # TODO automatically get from QHYCCD CMakeLists.txt
  set(LIBQHY_VERSION "0.1.8")
  set(LIBQHY_SOVERSION "0")

  #TODO replace this script invokation
  execute_process(COMMAND ${CMAKE_SOURCE_DIR}/scripts/qhy_fix_libusb_include ${CMAKE_CURRENT_BINARY_DIR}/qhyccdstruct.h)

  include_directories(${CMAKE_CURRENT_BINARY_DIR})

  configure_file(qhyexception.cpp.in qhyexception.cpp)
  set(qhy_GENERATED_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/qhyexception.cpp)
  file(GLOB qhy_SOURCES *.cpp)


  set(QHY_LIBRARY_armv6 ${CMAKE_CURRENT_SOURCE_DIR}/QHYCCD_Linux/libqhy_arm_v6.bin)
  set(QHY_LIBRARY_armv7 ${CMAKE_CURRENT_SOURCE_DIR}/QHYCCD_Linux/libqhy_arm_v7.bin)
  set(QHY_LIBRARY_x86_64 ${CMAKE_CURRENT_SOURCE_DIR}/QHYCCD_Linux/libqhy_64.bin)
  set(QHY_LIBRARY_i686 ${CMAKE_CURRENT_SOURCE_DIR}/QHYCCD_Linux/libqhy_32.bin)

  set(QHY_LIBRARY ${QHY_LIBRARY_${PlanetaryImager_ARCH}})


  # if(ADD_DRIVERS_BUILD_DIRECTORY)
  #   get_filename_component(QHY_LIBRARY_NAME ${QHY_LIBRARY} NAME)
  #   file(COPY ${QHY_LIBRARY} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
  #   file(RENAME ${CMAKE_CURRENT_BINARY_DIR}/${QHY_LIBRARY_NAME} ${CMAKE_CURRENT_BINARY_DIR}/libqhy.so.${LIBQHY_VERSION})
  # endif()


  install(FILES QHYCCD_Linux/85-qhy.rules DESTINATION ${UDEVRULES_INSTALL_DIR})
  install(FILES ${QHY_LIBRARY} DESTINATION ${drivers_destination} RENAME libqhy.so.${LIBQHY_VERSION})

  file(GLOB QHY_FIRMWARE_FILES QHYCCD_Linux/firmware/*)
  install(FILES ${QHY_FIRMWARE_FILES} DESTINATION ${FIRMWARE_INSTALL_BASEDIR}/qhy)
endif()
add_driver(NAME qhy OS Linux DEFAULT_ON SRCS ${qhy_SOURCES} ${qhy_GENERATED_SOURCES} LINK ${QHY_LIBRARY})
