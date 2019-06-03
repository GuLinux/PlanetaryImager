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

function(add_sdk)
  # Sets the following variables into cache:
  # ${NAME}_SDK_DIR
  # ${NAME}_SDK_VERSION
  # ${NAME}_SDK_URL

  set(options "")
  set(oneValueArgs NAME MAJOR MINOR PATCH SUB URL HASH HASH_ALGO)
  set(multiValueArgs "")
  cmake_parse_arguments(add_sdk "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  set(SDK_VERSION "${add_sdk_MAJOR}.${add_sdk_MINOR}.${add_sdk_PATCH}${add_sdk_SUB}")
  set(SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${add_sdk_NAME}_v${SDK_VERSION}")
  set(${add_sdk_NAME}_SDK_DIR "${SDK_DIR}" CACHE STRING "${add_sdk_NAME} SDK Directory")
  set(${add_sdk_NAME}_SDK_VERSION "${SDK_VERSION}" CACHE STRING "${add_sdk_NAME} SDK Version")

  string(CONFIGURE "${add_sdk_URL}" SDK_URL)

  set(${add_sdk_NAME}_SDK_URL ${SDK_URL} CACHE STRING "${add_sdk_NAME} SDK URL")
  get_filename_component(SDK_ARCHIVE "${SDK_URL}" NAME)
  set(SDK_ARCHIVE "${CMAKE_CURRENT_BINARY_DIR}/${SDK_ARCHIVE}")
  
  message("Adding `${add_sdk_NAME}` SDK: `${SDK_DIR}` `${SDK_URL}` `${SDK_ARCHIVE}`")

  if("${SDK_ARCHIVE}" MATCHES ".tar.bz2$")
    set(EXTRACT_COMMAND tar)
    set(EXTRACT_COMMAND_ARGS xjf ${SDK_ARCHIVE})
  elseif("${SDK_ARCHIVE}" MATCHES ".tar.gz$")
    set(EXTRACT_COMMAND tar)
    set(EXTRACT_COMMAND_ARGS xzf ${SDK_ARCHIVE})
  elseif("${SDK_ARCHIVE}" MATCHES ".zip$")
    set(EXTRACT_COMMAND unzip)
    set(EXTRACT_COMMAND_ARGS ${SDK_ARCHIVE})
  else()
    message(FATAL_ERROR "${SDK_ARCHIVE}: Unrecognized archive format")
  endif()

  if(NOT EXISTS ${SDK_DIR})
    message(STATUS "Downloading ${add_sdk_NAME} SDK v${SDK_VERSION}")
    file(DOWNLOAD
          "${SDK_URL}"
          "${SDK_ARCHIVE}"
          EXPECTED_HASH ${add_sdk_HASH_ALGO}=${add_sdk_HASH}
          SHOW_PROGRESS LOG SDK_DOWNLOAD_LOG STATUS SDK_DOWNLOAD_STATUS
    )
  
    list(LENGTH SDK_DOWNLOAD_STATUS SDK_DOWNLOAD_STATUS_LENGTH)
    list(GET SDK_DOWNLOAD_STATUS 0 SDK_DOWNLOAD_STATUS_CODE)
    list(GET SDK_DOWNLOAD_STATUS 1 SDK_DOWNLOAD_STATUS_MESSAGE)
    if(${SDK_DOWNLOAD_STATUS_CODE})
      message(FATAL_ERROR "Error while downloading ${add_sdk_NAME} SDK: ${SDK_DOWNLOAD_STATUS_MESSAGE}(code: ${SDK_DOWNLOAD_STATUS_MESSAGE}. Full log: ${SDK_DOWNLOAD_LOG}")
    endif()
    file(MAKE_DIRECTORY "${SDK_DIR}")
    execute_process(COMMAND ${EXTRACT_COMMAND} ${EXTRACT_COMMAND_ARGS} WORKING_DIRECTORY ${SDK_DIR} RESULT_VARIABLE UNPACK_ERROR)
    if(NOT ${UNPACK_ERROR} EQUAL 0)
      file(REMOVE_RECURSE ${SDK_DIR})
      message(FATAL_ERROR "Error unpacking ${add_sdk_NAME} SDK: ${UNPACK_ERROR}")
    endif()
  endif()


endfunction()
