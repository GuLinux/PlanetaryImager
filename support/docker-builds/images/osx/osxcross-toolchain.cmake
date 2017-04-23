set(CMAKE_SYSTEM_NAME Darwin)
set(CMAKE_OSX_SYSROOT /osxcross/target/SDK/MacOSX10.11.sdk/)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.11" CACHE STRING "Minimum OS X deployment version")
set(TOOLCHAIN_PREFIX x86_64-apple-darwin15)
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-clang)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-clang++)
set(CMAKE_FIND_ROOT_PATH "/osxcross/target/SDK/MacOSX10.11.sdk/usr/include/")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
set(CMAKE_FIND_ROOT_PATH "/osxcross/target/macports/pkgs/opt/local/")
set(CMAKE_INSTALL_FRAMEWORK_PREFIX "/osxcross/target/SDK/MacOSX10.11.sdk/System/Library/Frameworks")

set(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG "-Wl,-rpath,")
set(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG_SEP ":")
# more linking variables
set(CMAKE_INSTALL_NAME_DIR "@rpath")
set(CMAKE_INSTALL_RPATH ...)




#set(CMAKE_AR /usr/bin/ar CACHE FILEPATH "Archiver")
set(AM_QT_MOC_EXECUTABLE "/usr/lib/x86_64-linux-gnu/qt5/bin/moc")
set(QT_MOC_EXECUTABLE ${AM_QT_MOC_EXECUTABLE})
if(NOT TARGET Qt5::moc) 
    add_executable(Qt5::moc IMPORTED)
endif()
set_property(TARGET Qt5::moc PROPERTY IMPORTED_LOCATION ${QT_MOC_EXECUTABLE})

set(AM_QT_UIC_EXECUTABLE "/usr/lib/x86_64-linux-gnu/qt5/bin/uic")
set(QT_UIC_EXECUTABLE ${AM_QT_UIC_EXECUTABLE})
if(NOT TARGET Qt5::uic)
    add_executable(Qt5::uic IMPORTED)
endif()
set_property(TARGET Qt5::uic PROPERTY IMPORTED_LOCATION ${QT_UIC_EXECUTABLE})


set(AM_QT_RCC_EXECUTABLE "/usr/lib/x86_64-linux-gnu/qt5/bin/rcc")
set(QT_RCC_EXECUTABLE ${AM_QT_RCC_EXECUTABLE})
if(NOT TARGET Qt5::rcc)
    add_executable(Qt5::rcc IMPORTED)
endif()
set_property(TARGET Qt5::rcc PROPERTY IMPORTED_LOCATION ${QT_RCC_EXECUTABLE})

set(CCFITS_LIBRARY_PATH /opt/CCfits/lib/ CACHE PATH "CCfits find_library include path")
include_directories(/opt/CCfits/include/)
link_directories(/opt/CCfits/lib)
set(CMAKE_PREFIX_PATH "/osxcross/target/macports/pkgs/opt/local/libexec/qt5/;/osxcross/target/macports/pkgs/opt/local;/opt/CCfits;/opt/cfitsio")


# run with -DCMAKE_TOOLCHAIN_FILE=/osxcross-toolchain.cmake

