install(FILES
  /mxe/usr/x86_64-w64-mingw32.shared/bin/libbz2.dll
  /mxe/usr/x86_64-w64-mingw32.shared/bin/libfreetype-6.dll
  /mxe/usr/x86_64-w64-mingw32.shared/bin/libgcc_s_seh-1.dll
  /mxe/usr/x86_64-w64-mingw32.shared/bin/libglib-2.0-0.dll
  /mxe/usr/x86_64-w64-mingw32.shared/bin/libHalf-12.dll
  /mxe/usr/x86_64-w64-mingw32.shared/bin/libharfbuzz-0.dll
  /mxe/usr/x86_64-w64-mingw32.shared/bin/libiconv-2.dll
  /mxe/usr/x86_64-w64-mingw32.shared/bin/libIex-2_2-12.dll
  /mxe/usr/x86_64-w64-mingw32.shared/bin/libIlmImf-2_2-22.dll
  /mxe/usr/x86_64-w64-mingw32.shared/bin/libIlmThread-2_2-12.dll
  /mxe/usr/x86_64-w64-mingw32.shared/bin/libintl-8.dll
  /mxe/usr/x86_64-w64-mingw32.shared/bin/libjasper-1.dll
  /mxe/usr/x86_64-w64-mingw32.shared/bin/libjpeg-9.dll
  /mxe/usr/x86_64-w64-mingw32.shared/bin/liblzma-5.dll
  /mxe/usr/x86_64-w64-mingw32.shared/bin/libopencv_core.dll
  /mxe/usr/x86_64-w64-mingw32.shared/bin/libopencv_highgui.dll
  /mxe/usr/x86_64-w64-mingw32.shared/bin/libopencv_imgproc.dll
  /mxe/usr/x86_64-w64-mingw32.shared/bin/libpcre-1.dll
  /mxe/usr/x86_64-w64-mingw32.shared/bin/libpcre2-16-0.dll
  /mxe/usr/x86_64-w64-mingw32.shared/bin/libpcre-1.dll
  /mxe/usr/x86_64-w64-mingw32.shared/bin/libpng16-16.dll
  /mxe/usr/x86_64-w64-mingw32.shared/bin/libstdc++-6.dll
  /mxe/usr/x86_64-w64-mingw32.shared/bin/libtiff-5.dll
  /mxe/usr/x86_64-w64-mingw32.shared/qt5/bin/Qt5Core.dll
  /mxe/usr/x86_64-w64-mingw32.shared/qt5/bin/Qt5Gui.dll
  /mxe/usr/x86_64-w64-mingw32.shared/qt5/bin/Qt5OpenGL.dll
  /mxe/usr/x86_64-w64-mingw32.shared/qt5/bin/Qt5Widgets.dll
  /mxe/usr/x86_64-w64-mingw32.shared/qt5/bin/Qt5Qml.dll
  /mxe/usr/x86_64-w64-mingw32.shared/qt5/bin/Qt5Network.dll
  /mxe/usr/x86_64-w64-mingw32.shared/bin/zlib1.dll
  /mxe/usr/x86_64-w64-mingw32.shared/bin/libeay32.dll
  /mxe/usr/x86_64-w64-mingw32.shared/bin/ssleay32.dll
DESTINATION .)
file(GLOB QT_PLUGINS /mxe/usr/x86_64-w64-mingw32.shared/qt5/plugins/*)
install(DIRECTORY ${QT_PLUGINS} DESTINATION .)
