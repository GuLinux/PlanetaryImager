set(MXE_USR /mxe/usr)
set(MXE_MINGW ${MXE_USR}/x86_64-w64-mingw32.shared)

add_custom_command(
    TARGET planetary_imager
    PRE_BUILD
    COMMAND rm -rf ${CMAKE_BINARY_DIR}/deps
    COMMAND mkdir -p ${CMAKE_BINARY_DIR}/deps
    COMMENT Remove deps directory
)


add_custom_command(
    TARGET planetary_imager
    POST_BUILD
    COMMAND /mxe/tools/copydlldeps.sh -p -c -f ${CMAKE_BINARY_DIR}/src/planetary_imager.exe  -d ${CMAKE_BINARY_DIR}/deps/ -s ${MXE_MINGW}/lib/ -s ${MXE_MINGW}/bin/ -s ${MXE_MINGW}/qt5/bin/
    COMMENT Copying dependencies for planetary_imager
)

if(build_network_server)
    add_custom_command(
        TARGET planetary_imager_frontend
        POST_BUILD
        COMMAND /mxe/tools/copydlldeps.sh -p -c -f ${CMAKE_BINARY_DIR}/src/planetary_imager_frontend.exe  -d ${CMAKE_BINARY_DIR}/deps/ -s ${MXE_MINGW}/lib/ -s ${MXE_MINGW}/bin/ -s ${MXE_MINGW}/qt5/bin/
        COMMENT Copying dependencies for planetary_imager_frontend
    )

    add_custom_command(
        TARGET planetary_imager_daemon
        POST_BUILD
        COMMAND /mxe/tools/copydlldeps.sh -p -c -f ${CMAKE_BINARY_DIR}/src/planetary_imager_daemon.exe  -d ${CMAKE_BINARY_DIR}/deps/ -s ${MXE_MINGW}/lib/ -s ${MXE_MINGW}/bin/ -s ${MXE_MINGW}/qt5/bin/
        COMMENT Copying dependencies for planetary_imager_daemon
    )
endif()

install(
    DIRECTORY ${CMAKE_BINARY_DIR}/deps/
    DESTINATION .
)

file(GLOB QT_PLUGINS ${MXE_MINGW}/qt5/plugins/*)
install(DIRECTORY ${QT_PLUGINS} DESTINATION .)
