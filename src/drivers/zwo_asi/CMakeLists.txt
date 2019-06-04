file(GLOB asi_imager_SRCS *.cpp)

add_sdk(NAME ASI_Linux_Mac MAJOR 1 MINOR 14 PATCH 0425 URL "http://astronomy-imaging-camera.com/software/ASI_linux_mac_SDK_V\${ASI_Linux_Mac_SDK_VERSION}.tar.bz2" HASH_ALGO MD5 HASH f1faced49f0aca0b8ebf34feb15eec3b) 
add_sdk(NAME ASI_Windows   MAJOR 1 MINOR 14 PATCH 0425 URL "http://astronomy-imaging-camera.com/software/ASI_Windows_SDK_V\${ASI_Windows_SDK_VERSION}.zip" HASH_ALGO MD5 HASH b33e384377f6862fbbf623a0450d2488) 

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(ASI_SDK_VERSION ${ASI_Windows_SDK_VERSION})
    set(ASI_SDK_DIR ${ASI_Windows_SDK_DIR})
    set(asi_libfilename "ASICamera2.dll")
else()
    set(ASI_SDK_VERSION ${ASI_Linux_Mac_SDK_VERSION})
    set(ASI_SDK_DIR ${ASI_Linux_Mac_SDK_DIR})
    set(asi_libfilename "libASICamera2.a")
endif()

add_definitions(-DASI_SDK_VERSION="${ASI_SDK_VERSION}")
set(ASI_ARCH_x86_64 x64)
set(ASI_ARCH_i686 x86)
set(ASI_ARCH_armv8 armv8)
set(ASI_ARCH_armv7 armv7)
set(ASI_ARCH_armv6 armv6)

set(ASI_TARGET_ARCH ${ASI_ARCH_${PlanetaryImager_ARCH}})
if( "${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    set(ASI_TARGET_ARCH "mac")
endif()

include_directories(${ASI_SDK_DIR}/include)
set(asi_driver_library ${ASI_SDK_DIR}/lib/${ASI_TARGET_ARCH}/${asi_libfilename})


add_driver(NAME zwo_asi_imager OS Linux Darwin Windows DEFAULT_ON SRCS ${asi_imager_SRCS} LINK ${asi_driver_library})

if(BUILD_DRIVER_zwo_asi_imager)
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        install(FILES ${ASI_SDK_DIR}/lib/asi.rules DESTINATION ${UDEVRULES_INSTALL_DIR} RENAME 97-asi.rules) # Use a different name than INDI, to avoid package conflicts
    endif()
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux" OR ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
        target_link_libraries(zwo_asi_imager -lusb-1.0)
    endif()

    if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
        target_link_libraries(zwo_asi_imager "-framework CoreFoundation" "-framework IOKit" -lobjc)
    endif()

    if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
        install(FILES ${asi_driver_library} DESTINATION .)
    endif()
endif()
