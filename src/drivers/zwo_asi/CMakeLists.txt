add_library(zwo_asi_imager STATIC zwo_asi_driver.cpp zwo_asi_imager.cpp zwoexception.cpp asiimagingworker.cpp asicontrol.cpp)
set(ZWO_SDK_VERSION 0.3.0819)
#set(ZWO_SDK_VERSION 0.3.0727)
set(ZWO_SDK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ASI_SDK_v${ZWO_SDK_VERSION}/)

set(ZWO_ARCH_x86_64 x64)
set(ZWO_ARCH_i686 x86)
set(ZWO_ARCH_armv7 armv7)
set(ZWO_ARCH_armv6 armv6)

set(ZWO_TARGET_ARCH ${ZWO_ARCH_${PlanetaryImager_ARCH}})

include_directories(${ZWO_SDK_DIR}/include)

add_imager_dependencies(zwo_asi_imager ${ZWO_SDK_DIR}/lib/${ZWO_TARGET_ARCH}/libASICamera2.a -lusb-1.0)
define_driver(drivers/zwo_asi/zwo_asi_driver.h ZWO_ASI_Driver)

install(FILES ${ZWO_SDK_DIR}/lib/asi.rules DESTINATION ${UDEVRULES_INSTALL_DIR})
