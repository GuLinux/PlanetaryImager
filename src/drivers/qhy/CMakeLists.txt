if(CMAKE_SIZEOF_VOID_P MATCHES "8")
  set(QHY_LIBRARY ${CMAKE_SOURCE_DIR}/QHYCCD_Linux/libqhy_64.bin)
else(CMAKE_SIZEOF_VOID_P MATCHES "8")
  set(QHY_LIBRARY ${CMAKE_SOURCE_DIR}/QHYCCD_Linux/libqhy_32.bin)
endif(CMAKE_SIZEOF_VOID_P MATCHES "8")


# TODO: remove png and jpeg from here, it's temporary until issue https://github.com/qhyccd-lzr/QHYCCD_Linux/issues/12 is solved
set(planetary_imager_DEPS ${planetary_imager_DEPS} png jpeg qhy_driver ${QHY_LIBRARY} CACHE INTERNAL "qhy dependencies" )
set(qhy_SOURCES qhydriver.cpp qhyccdimager.cpp)
configure_file(qhy_messages.in qhy_messages.cpp)
add_library(qhy_driver STATIC ${qhy_SOURCES})
