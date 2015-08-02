find_package(Qt5Test)
add_executable(test_qimage_destructor test_qimage_destructor.cpp)
target_link_libraries(test_qimage_destructor Qt5::Widgets Qt5::Test)