include(${CMAKE_CURRENT_LIST_DIR}/configuration-debian-base.cmake)
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt5gui5 (>= 5.5.0), libqt5opengl5 (>= 5.5.0), libqt5network5 (>= 5.5.0), libqt5widgets5 (>= 5.5.0), libopencv-imgproc2.4v5, libopencv-core2.4v5, libopencv-highgui2.4-deb0, libccfits0v5, libopencv-videostab2.4v5, libopencv-ts2.4v5, libopencv-superres2.4v5, libopencv-stitching2.4v5, libopencv-contrib2.4v5, libqt5qml5" CACHE STRING "")
set(EXTRA_LIBRARIES udev CACHE STRING "")
