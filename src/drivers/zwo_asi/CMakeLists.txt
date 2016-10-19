file(GLOB asi_imager_SRCS *.cpp)
set(ASI_SDK_MAJOR 0)
set(ASI_SDK_MINOR 4)
set(ASI_SDK_PATCH 0920)
set(ASI_SDK_VERSION ${ASI_SDK_MAJOR}.${ASI_SDK_MINOR}.${ASI_SDK_PATCH})
set(ASI_SDK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ASI_SDK_v${ASI_SDK_VERSION}/)
if(ASI_SDK_VERSION VERSION_GREATER 0.4.0918)
  add_definitions(-DASI_CAMERA_REQUIRES_INIT)
endif(ASI_SDK_VERSION VERSION_GREATER 0.4.0918)
add_definitions(-DASI_SDK_VERSION="${ASI_SDK_VERSION}")
set(ASI_ARCH_x86_64 x64)
set(ASI_ARCH_i686 x86)
set(ASI_ARCH_armv7 armv7)
set(ASI_ARCH_armv6 armv6)

set(ASI_TARGET_ARCH ${ASI_ARCH_${PlanetaryImager_ARCH}})

include_directories(${ASI_SDK_DIR}/include)

add_imager_dependencies(zwo_asi_imager ${ASI_SDK_DIR}/lib/${ASI_TARGET_ARCH}/libASICamera2.a -lusb-1.0)
define_driver(drivers/zwo_asi/zwo_asi_driver.h ZWO_ASI_Driver)

add_library(zwo_asi_imager STATIC ${asi_imager_SRCS})
install(FILES ${ASI_SDK_DIR}/lib/asi.rules DESTINATION ${UDEVRULES_INSTALL_DIR})
