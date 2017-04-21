set(CPACK_RPM_PACKAGE_REQUIRES "libusb, qt5-qtbase >= 5.5.0, opencv >= 2.4.0 , fxload, CCfits")
set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST ${CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST} /lib /lib/udev /lib/udev/rules.d /usr/bin /usr/lib)
set(CPACK_RPM_PACKAGE_AUTOREQ 0)
