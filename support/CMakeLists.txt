include_directories(${CMAKE_SOURCE_DIR})
add_executable(ser_extract_frames ser_extract_frames.cpp)
target_link_libraries(ser_extract_frames Qt5::Core)