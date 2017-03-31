file(GLOB asi_imager_SRCS *.cpp)
set(ASI_LINUX_MAC_SDK_MAJOR 0)
set(ASI_LINUX_MAC_SDK_MINOR 6)
set(ASI_LINUX_MAC_SDK_PATCH 0328)
set(ASI_LINUX_MAC_SDK_VERSION ${ASI_LINUX_MAC_SDK_MAJOR}.${ASI_LINUX_MAC_SDK_MINOR}.${ASI_LINUX_MAC_SDK_PATCH})
set(ASI_LINUX_MAC_SDK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ASI_LINUX_MAC_SDK_v${ASI_LINUX_MAC_SDK_VERSION}/)

set(ASI_WINDOWS_SDK_MAJOR 1)
set(ASI_WINDOWS_SDK_MINOR 13)
set(ASI_WINDOWS_SDK_PATCH 0)
set(ASI_WINDOWS_SDK_PATCH_SUB 10)
set(ASI_WINDOWS_SDK_VERSION ${ASI_WINDOWS_SDK_MAJOR}.${ASI_WINDOWS_SDK_MINOR}.${ASI_WINDOWS_SDK_PATCH}.${ASI_WINDOWS_SDK_PATCH_SUB})
set(ASI_WINDOWS_SDK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ASI_WINDOWS_SDK_v${ASI_WINDOWS_SDK_VERSION}/)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(ASI_SDK_VERSION ${ASI_WINDOWS_SDK_VERSION})
    set(ASI_SDK_DIR ${ASI_WINDOWS_SDK_DIR})
    set(asi_libfilename "ASICamera2.dll")
else()
    set(ASI_SDK_VERSION ${ASI_LINUX_MAC_SDK_VERSION})
    set(ASI_SDK_DIR ${ASI_LINUX_MAC_SDK_DIR})
    set(asi_libfilename "libASICamera2.a")
endif()


add_definitions(-DASI_SDK_VERSION="${ASI_SDK_VERSION}")
set(ASI_ARCH_x86_64 x64)
set(ASI_ARCH_i686 x86)
set(ASI_ARCH_armv7 armv7)
set(ASI_ARCH_armv6 armv6)

set(ASI_TARGET_ARCH ${ASI_ARCH_${PlanetaryImager_ARCH}})
if( "${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    set(ASI_TARGET_ARCH "mac")
endif()

include_directories(${ASI_SDK_DIR}/include)
set(asi_driver_library ${ASI_SDK_DIR}/lib/${ASI_TARGET_ARCH}/${asi_libfilename})

if(NOT EXISTS ${ASI_LINUX_MAC_SDK_DIR})
  message(STATUS "Downloading ASI Linux/Mac SDK v${ASI_LINUX_MAC_SDK_VERSION}")
  file(DOWNLOAD
        http://astronomy-imaging-camera.com/software/ASI_linux_mac_SDK_V${ASI_LINUX_MAC_SDK_VERSION}.tar
        ${CMAKE_CURRENT_SOURCE_DIR}/asi_sdk.tar
        SHOW_PROGRESS LOG asi_sdk_download_log STATUS asi_sdk_download_status)

  list(GET ${asi_sdk_download_status} 0 asi_sdk_download_status_code)
  list(GET ${asi_sdk_download_status} 1 asi_sdk_download_status_message)
  if(${asi_sdk_download_status_code})
    message(FATAL_ERROR "Error while downloading ASI Linux/Mac SDK: ${asi_sdk_download_status_message}(code: ${asi_sdk_download_status_message}. Full log: ${asi_sdk_download_log}")
  endif()
  message(STATUS "ASI Linux/Mac SDK downloaded, extracting")
  execute_process(COMMAND tar xvf asi_sdk.tar WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  file(REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/asi_sdk.tar)
  file(MAKE_DIRECTORY ${ASI_LINUX_MAC_SDK_DIR})
  execute_process(COMMAND tar xf ASI_linux_mac_SDK.tar.bz2 -C ${ASI_LINUX_MAC_SDK_DIR} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  file(REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/ASI_linux_mac_SDK.tar.bz2)
endif()

if(NOT EXISTS ${ASI_WINDOWS_SDK_DIR})
  message(STATUS "Downloading ASI Windows SDK v${ASI_WINDOWS_SDK_VERSION}")
  file(DOWNLOAD
         http://astronomy-imaging-camera.com/software/ASI_Windows_SDK_V${ASI_WINDOWS_SDK_VERSION}.zip
         ${CMAKE_CURRENT_SOURCE_DIR}/asi_sdk.zip
         SHOW_PROGRESS LOG asi_sdk_download_log STATUS asi_sdk_download_status)
 
  list(GET ${asi_sdk_download_status} 0 asi_sdk_download_status_code)
  list(GET ${asi_sdk_download_status} 1 asi_sdk_download_status_message)
  if(${asi_sdk_download_status_code})
    message(FATAL_ERROR "Error while downloading ASI Windows SDK: ${asi_sdk_download_status_message}(code: ${asi_sdk_download_status_message}. Full log: ${asi_sdk_download_log}")
  endif()
  message(STATUS "ASI Windows SDK downloaded, extracting")
  file(MAKE_DIRECTORY ${ASI_WINDOWS_SDK_DIR})
  execute_process(COMMAND unzip asi_sdk.zip WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  file(RENAME "${CMAKE_CURRENT_SOURCE_DIR}/ASI SDK" ${ASI_WINDOWS_SDK_DIR})
  file(REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/asi_sdk.zip)
endif()



add_driver(NAME zwo_asi_imager OS Linux Darwin Windows DEFAULT_ON SRCS ${asi_imager_SRCS} LINK ${asi_driver_library})

if(BUILD_DRIVER_zwo_asi_imager)
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        install(FILES ${ASI_SDK_DIR}/lib/asi.rules DESTINATION ${UDEVRULES_INSTALL_DIR})
    endif()
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux" OR ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
        target_link_libraries(zwo_asi_imager -lusb-1.0)
    endif()

    if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
        install(FILES ${asi_driver_library} DESTINATION .)
    endif()
endif()
