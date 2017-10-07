include_directories(googletest/include)
add_executable(test_qimage_destructor test_qimage_destructor.cpp)
target_link_libraries(test_qimage_destructor gtest_main Qt5::Widgets)
add_test(qimage_destructor test_qimage_destructor)

add_executable(test_roi_validator test_roi_validator.cpp)
target_link_libraries(test_roi_validator gtest_main drivers Qt5::Widgets)
add_test(roi_validator_test test_roi_validator)

add_executable(test_ser_header test_ser_header.cpp ${CMAKE_SOURCE_DIR}/src/commons/ser_header.cpp)
target_link_libraries(test_ser_header gtest_main ${OpenCV_LIBS} Qt5::Widgets)
add_test(ser_header_test test_ser_header)

add_executable(test_frame test_frame.cpp ${CMAKE_SOURCE_DIR}/src/commons/frame.cpp)
target_link_libraries(test_frame gtest_main opencv_core Qt5::Widgets)
add_test(test_frame test_frame)

add_executable(test_networkpacket test_networkpacket.cpp ${CMAKE_SOURCE_DIR}/src/network/networkpacket.cpp)
target_link_libraries(test_networkpacket gtest_main ${OpenCV_LIBS} Qt5::Widgets)
add_test(test_networkpacket test_networkpacket)


add_subdirectory(googletest)
