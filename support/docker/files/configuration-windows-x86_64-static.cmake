set(cflags_fpic Off CACHE BOOL "")
set(static_qt_windows On CACHE BOOL "")
set(CMAKE_INSTALL_PREFIX / CACHE PATH "")
set(CPACK_GENERATOR ZIP CACHE STRING "")

set(MXE_ARCH x86_64)
set(MXE_ROOT /mxe/usr/${MXE_ARCH}-w64-mingw32.static/)

execute_process(COMMAND /cmake/get-libs-from-qt-prl ${MXE_ROOT}/qt5/plugins/platforms/qwindows.prl ${MXE_ROOT}/qt5/lib OUTPUT_VARIABLE QWINDOWS_LIBS)
set(EXTRA_LIBRARIES
    Qt5::QWindowsIntegrationPlugin
    ${QWINDOWS_LIBS}
    ssl
    crypto
    mincore
    CACHE STRING ""
)
