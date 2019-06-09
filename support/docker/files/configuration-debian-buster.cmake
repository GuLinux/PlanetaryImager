include(${CMAKE_CURRENT_LIST_DIR}/configuration-debian-base.cmake)
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libusb-1.0-0, libqt5gui5 (>= 5.5.0), libqt5opengl5 (>= 5.5.0), libqt5network5 (>= 5.5.0), libqt5widgets5 (>= 5.5.0), libopencv-imgproc3.2, libopencv-core3.2, libopencv-highgui3.2, fxload, libccfits0v5, libopencv-shape3.2, libopencv-stitching3.2, libopencv-superres3.2, libopencv-videostab3.2, libopencv-contrib3.2, libqt5qml5" CACHE STRING "")
set(EXTRA_LIBRARIES udev CACHE STRING "")
