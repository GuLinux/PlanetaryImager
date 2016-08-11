add_library(zwo_asi_imager STATIC zwo_asi_driver.cpp zwo_asi_imager.cpp )
#set(ZWO_SDK_VERSION 0.3.0623)
set(ZWO_SDK_VERSION 0.3.0727)
set(ZWO_SDK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ASI_linux_mac_SDK_V${ZWO_SDK_VERSION}/ASI_linux_mac_SDK)

set(ZWO_TARGET_ARCH "x64" CACHE STRING "ZWO Architecture (default: x64, look at ${ZWO_SDK_DIR}/lib for possible values)")

include_directories(${ZWO_SDK_DIR}/include)
add_imager_dependencies(zwo_asi_imager ${ZWO_SDK_DIR}/lib/${ZWO_TARGET_ARCH}/libASICamera2.a -lusb-1.0)
define_driver(drivers/zwo_asi/zwo_asi_driver.h ZWO_ASI_Driver)

set(ASI_UDEVRULES_INSTALL_DIR "/lib/udev/rules.d" CACHE STRING "Base directory for udev rules")
install(FILES ${ZWO_SDK_DIR}/lib/asi.rules DESTINATION ${ASI_UDEVRULES_INSTALL_DIR})
