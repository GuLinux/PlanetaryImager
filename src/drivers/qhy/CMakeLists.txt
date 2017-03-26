execute_process(COMMAND ${CMAKE_SOURCE_DIR}/scripts/qhy_errors_to_map ${CMAKE_CURRENT_SOURCE_DIR}/QHYCCD_Linux/qhyccderr.h OUTPUT_VARIABLE qhy_error_codes)

configure_file(QHYCCD_Linux/qhyccd.h ${CMAKE_CURRENT_BINARY_DIR}/ COPYONLY)
configure_file(QHYCCD_Linux/qhyccdcamdef.h ${CMAKE_CURRENT_BINARY_DIR}/ COPYONLY)
configure_file(QHYCCD_Linux/qhyccdstruct.h ${CMAKE_CURRENT_BINARY_DIR}/ COPYONLY)
configure_file(QHYCCD_Linux/qhyccderr.h ${CMAKE_CURRENT_BINARY_DIR}/ COPYONLY)
execute_process(COMMAND ${CMAKE_SOURCE_DIR}/scripts/qhy_fix_libusb_include ${CMAKE_CURRENT_BINARY_DIR}/qhyccdstruct.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

configure_file(qhyexception.cpp.in qhyexception.cpp)
set(qhy_GENERATED_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/qhyexception.cpp)
file(GLOB qhy_SOURCES *.cpp)


set(QHY_LIBRARY_armv6 ${CMAKE_CURRENT_SOURCE_DIR}/QHYCCD_Linux/libqhy_arm_v6.a)
set(QHY_LIBRARY_armv7 ${CMAKE_CURRENT_SOURCE_DIR}/QHYCCD_Linux/libqhy_arm_v7.a)
set(QHY_LIBRARY_x86_64 ${CMAKE_CURRENT_SOURCE_DIR}/QHYCCD_Linux/libqhy_64.a)
set(QHY_LIBRARY_i686 ${CMAKE_CURRENT_SOURCE_DIR}/QHYCCD_Linux/libqhy_32.a)

set(QHY_LIBRARY ${QHY_LIBRARY_${PlanetaryImager_ARCH}})
install(FILES QHYCCD_Linux/85-qhy.rules DESTINATION ${UDEVRULES_INSTALL_DIR})
file(GLOB QHY_FIRMWARE_FILES QHYCCD_Linux/firmware/*)
install(FILES ${QHY_FIRMWARE_FILES} DESTINATION ${FIRMWARE_INSTALL_BASEDIR}/qhy)

add_driver(NAME qhy OS Linux SRCS ${qhy_SOURCES} ${qhy_GENERATED_SOURCES} LINK ${QHY_LIBRARY})
