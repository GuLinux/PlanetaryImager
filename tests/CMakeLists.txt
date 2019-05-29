include_directories(googletest/include)

add_pi_test(NAME qimage_destructor SRCS test_qimage_destructor.cpp)
add_pi_test(NAME roi_validator SRCS test_roi_validator.cpp TARGET_LINK_LIBRARIES drivers)
add_pi_test(NAME ser_header SRCS test_ser_header.cpp ${CMAKE_SOURCE_DIR}/src/commons/ser_header.cpp TARGET_LINK_LIBRARIES ${OpenCV_LIBS})
add_pi_test(NAME frame SRCS test_frame.cpp ${CMAKE_SOURCE_DIR}/src/commons/frame.cpp TARGET_LINK_LIBRARIES opencv_core)
add_pi_test(NAME networkpacket SRCS test_networkpacket.cpp ${CMAKE_SOURCE_DIR}/src/network/networkpacket.cpp TARGET_LINK_LIBRARIES ${OpenCV_LIBS})


add_subdirectory(googletest)
