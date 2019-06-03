configure_file(${CMAKE_CURRENT_SOURCE_DIR}/macdeployqt ${CMAKE_CURRENT_BINARY_DIR}/macdeployqt)
install(CODE "
set(BUNDLE_PATH \$ENV\{DESTDIR\})
message(\"Running ${CMAKE_CURRENT_BINARY_DIR}/macdeployqt ${MACOSX_BUNDLE_BUNDLE_NAME}.app WORKING_DIRECTORY \${BUNDLE_PATH}\")
execute_process(COMMAND bash ${CMAKE_CURRENT_BINARY_DIR}/macdeployqt ${MACOSX_BUNDLE_BUNDLE_NAME}.app WORKING_DIRECTORY \${BUNDLE_PATH} )
message(\"Running ${CMAKE_SOURCE_DIR}/support/osx/fix_libraries.py WORKING_DIRECTORY \${BUNDLE_PATH}/${MACOSX_BUNDLE_BUNDLE_NAME}.app/Contents/Frameworks\")
execute_process(COMMAND python ${CMAKE_SOURCE_DIR}/support/osx/fix_libraries.py WORKING_DIRECTORY \${BUNDLE_PATH}/${MACOSX_BUNDLE_BUNDLE_NAME}.app/Contents/Frameworks )
")

