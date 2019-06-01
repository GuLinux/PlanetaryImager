configure_file(${CMAKE_CURRENT_SOURCE_DIR}/macdeployqt ${CMAKE_CURRENT_BINARY_DIR}/macdeployqt)
# install(CODE "
# execute_process(COMMAND bash ${CMAKE_CURRENT_BINARY_DIR}/macdeployqt ${MACOSX_BUNDLE_BUNDLE_NAME}.app WORKING_DIRECTORY \$ENV\{DESTDIR\} )
# ")

