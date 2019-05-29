include_directories(googletest/include)
include(GoogleTest)



add_custom_target(build_tests)

function(add_pi_test)
  set(options "")
  set(oneValueArgs NAME)
  set(multiValueArgs SRCS TARGET_LINK_LIBRARIES)
  cmake_parse_arguments(add_pi_test "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
 
  add_executable(test_${add_pi_test_NAME} ${add_pi_test_SRCS})
  target_link_libraries(test_${add_pi_test_NAME} gtest_main Qt5::Widgets ${add_pi_test_TARGET_LINK_LIBRARIES})
  #add_test(${add_pi_test_NAME} test_${add_pi_test_NAME})
  gtest_add_tests(TARGET test_${add_pi_test_NAME})
  add_dependencies(build_tests test_${add_pi_test_NAME})
endfunction()


add_pi_test(NAME qimage_destructor SRCS test_qimage_destructor.cpp)
add_pi_test(NAME roi_validator SRCS test_roi_validator.cpp TARGET_LINK_LIBRARIES drivers)
add_pi_test(NAME ser_header SRCS test_ser_header.cpp ${CMAKE_SOURCE_DIR}/src/commons/ser_header.cpp TARGET_LINK_LIBRARIES ${OpenCV_LIBS})
add_pi_test(NAME frame SRCS test_frame.cpp ${CMAKE_SOURCE_DIR}/src/commons/frame.cpp TARGET_LINK_LIBRARIES opencv_core)
add_pi_test(NAME networkpacket SRCS test_networkpacket.cpp ${CMAKE_SOURCE_DIR}/src/network/networkpacket.cpp TARGET_LINK_LIBRARIES ${OpenCV_LIBS})

external_project_download(GoogleTest.cmake.in googletest)
  
# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                 ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)

