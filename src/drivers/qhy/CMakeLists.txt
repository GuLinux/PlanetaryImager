execute_process(COMMAND ${CMAKE_SOURCE_DIR}/scripts/qhy_errors_to_map ${CMAKE_CURRENT_SOURCE_DIR}/QHYCCD_Linux/qhyccderr.h OUTPUT_VARIABLE qhy_error_codes)
execute_process(COMMAND ${CMAKE_SOURCE_DIR}/scripts/qhy_devices_to_map ${CMAKE_CURRENT_SOURCE_DIR}/QHYCCD_Linux/qhyccdcamdef.h OUTPUT_VARIABLE qhy_devices_codes)
include_directories(QHYCCD_Linux/)

set(qhy_SOURCES qhydriver.cpp qhyccdimager.cpp)

configure_file(qhy_messages.in qhy_messages.cpp)
add_library(qhy_driver STATIC ${qhy_SOURCES})

if(CMAKE_SIZEOF_VOID_P MATCHES "8")
  set(QHY_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/QHYCCD_Linux/libqhy_64.bin)
else(CMAKE_SIZEOF_VOID_P MATCHES "8")
  set(QHY_LIBRARY ${CMAKE__CURRENT_SOURCE_DIR}/QHYCCD_Linux/libqhy_32.bin)
endif(CMAKE_SIZEOF_VOID_P MATCHES "8")

add_imager_dependencies(qhy_driver ${QHY_LIBRARY})
define_driver(drivers/qhy/qhydriver.h QHYDriver)
add_subdirectory(QHYCCD_Linux/)