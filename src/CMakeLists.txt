set(QHYImager_SRCS QHYImager.cpp main.cpp qhydriver.cpp)

add_executable(qhyimager ${QHYImager_SRCS})

if(CMAKE_SIZEOF_VOID_P MATCHES "8")
  set(QHY_LIBRARY ${CMAKE_SOURCE_DIR}/QHYCCD_Linux/libqhy_64.bin)
else(CMAKE_SIZEOF_VOID_P MATCHES "8")
  set(QHY_LIBRARY ${CMAKE_SOURCE_DIR}/QHYCCD_Linux/libqhy_32.bin)
endif(CMAKE_SIZEOF_VOID_P MATCHES "8")


target_link_libraries(qhyimager Qt5::Widgets ${QHY_LIBRARY})

install(TARGETS qhyimager RUNTIME DESTINATION bin)
