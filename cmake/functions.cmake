# Dependencies management
unset(planetary_imager_commons_DEPS CACHE)
unset(planetary_imager_backend_DEPS CACHE)
unset(planetary_imager_frontend_DEPS CACHE)

function(add_imager_dependencies)
  set(planetary_imager_commons_DEPS ${planetary_imager_commons_DEPS} ${ARGN} CACHE INTERNAL "planetary imager common deps" FORCE)
endfunction()

function(add_backend_dependencies)
  set(planetary_imager_backend_DEPS ${planetary_imager_backend_DEPS} ${ARGN} CACHE INTERNAL "planetary imager backend deps" FORCE)
endfunction()
function(add_frontend_dependencies)
  set(planetary_imager_frontend_DEPS ${planetary_imager_frontend_DEPS} ${ARGN} CACHE INTERNAL "planetary imager frontend deps" FORCE)
endfunction()

# Drivers defintion
set(enabled_drivers "" CACHE INTERNAL "" FORCE)
set(disabled_drivers "" CACHE INTERNAL "" FORCE)
set(unsupported_drivers "" CACHE INTERNAL "" FORCE)

function(add_driver)
  set(options DEFAULT_ON)
  set(oneValueArgs NAME)
  set(multiValueArgs SRCS LINK OS)
  cmake_parse_arguments(add_driver "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  list(FIND add_driver_OS ${CMAKE_SYSTEM_NAME} OS_SUPPORTED)
  if(OS_SUPPORTED EQUAL -1)
    set(unsupported_drivers ${unsupported_drivers} ${add_driver_NAME} CACHE INTERNAL "")
    return()
  endif()
  option(BUILD_DRIVER_${add_driver_NAME} "Enable compilation of driver ${add_driver_NAME}" ${add_driver_DEFAULT_ON})
  if(NOT BUILD_DRIVER_${add_driver_NAME})
    set(disabled_drivers ${disabled_drivers} ${add_driver_NAME} CACHE INTERNAL "")
    return()
  endif()
  set(DRIVER_JSON_FILE ${CMAKE_CURRENT_BINARY_DIR}/driver_${add_driver_NAME}.json)

  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/driver.json ${DRIVER_JSON_FILE})
  add_library(${add_driver_NAME} MODULE ${add_driver_SRCS})
  target_link_libraries(${add_driver_NAME} GuLinux_Qt_Commons GuLinux_c++_Commons drivers planetaryimager-commons ${add_driver_LINK} Qt5::Core Qt5::Qml ${OpenCV_LIBS})

  set_target_properties(${add_driver_NAME} PROPERTIES PREFIX "driver_")

  install(FILES ${DRIVER_JSON_FILE} DESTINATION ${drivers_destination})
  install(TARGETS ${add_driver_NAME} LIBRARY DESTINATION ${drivers_destination})
  set(enabled_drivers ${enabled_drivers} ${add_driver_NAME} CACHE INTERNAL "")
endfunction()


add_custom_target(build_tests)

function(add_pi_test)
  set(options "")
  set(oneValueArgs NAME)
  set(multiValueArgs SRCS TARGET_LINK_LIBRARIES)
  cmake_parse_arguments(add_pi_test "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
 
  add_executable(test_${add_pi_test_NAME} ${add_pi_test_SRCS})
  target_link_libraries(test_${add_pi_test_NAME} gtest_main Qt5::Widgets ${add_pi_test_TARGET_LINK_LIBRARIES})
  add_test(${add_pi_test_NAME} test_${add_pi_test_NAME})
  add_dependencies(build_tests test_${add_pi_test_NAME})
endfunction()

function(external_project_download IN_FILE OUT_DIR)
  configure_file(${IN_FILE} ${OUT_DIR}_download/CMakeLists.txt)
  execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${OUT_DIR}_download)
  if(result)
    message(FATAL_ERROR "CMake step for ${OUT_DIR} failed: ${result}")
  endif()
  execute_process(COMMAND ${CMAKE_COMMAND} --build .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${OUT_DIR}_download )
  if(result)
    message(FATAL_ERROR "Build step for ${OUT_DIR} failed: ${result}")
  endif()
endfunction()

