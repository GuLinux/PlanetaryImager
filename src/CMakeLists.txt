execute_process(COMMAND ${CMAKE_SOURCE_DIR}/scripts/qhy_errors_to_map ${CMAKE_SOURCE_DIR}/QHYCCD_Linux/qhyccderr.h OUTPUT_VARIABLE qhy_error_codes)
execute_process(COMMAND ${CMAKE_SOURCE_DIR}/scripts/qhy_devices_to_map ${CMAKE_SOURCE_DIR}/QHYCCD_Linux/qhyccdcamdef.h OUTPUT_VARIABLE qhy_devices_codes)
configure_file(qhy_messages.in qhy_messages.cpp)

set(QHYImager_SRCS main.cpp qhydriver.cpp qhymainwindow.cpp qhymainwindow.ui qhyccdimager.cpp)

add_executable(qhyimager ${QHYImager_SRCS})

if(CMAKE_SIZEOF_VOID_P MATCHES "8")
  set(QHY_LIBRARY ${CMAKE_SOURCE_DIR}/QHYCCD_Linux/libqhy_64.bin)
else(CMAKE_SIZEOF_VOID_P MATCHES "8")
  set(QHY_LIBRARY ${CMAKE_SOURCE_DIR}/QHYCCD_Linux/libqhy_32.bin)
endif(CMAKE_SIZEOF_VOID_P MATCHES "8")


target_link_libraries(qhyimager Qt5::Widgets ${QHY_LIBRARY})

install(TARGETS qhyimager RUNTIME DESTINATION bin)
