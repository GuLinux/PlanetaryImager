include(${CMAKE_CURRENT_LIST_DIR}/configuration-fedora-base.cmake)
set(CPACK_RPM_PACKAGE_REQUIRES "qt5-qtbase >= 5.5.0, fxload, CCfits, libdc1394" CACHE STRING "")

