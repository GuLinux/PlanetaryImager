include(${CMAKE_CURRENT_LIST_DIR}/configuration-fedora-base.cmake)
set(CPACK_RPM_PACKAGE_REQUIRES "libusb, qt5-qtbase >= 5.5.0, opencv >= 2.4.0 , fxload, CCfits" CACHE STRING "")

