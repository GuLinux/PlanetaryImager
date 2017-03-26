file(GLOB testing_imager_SRCS *.cpp *.qrc)
add_library(testing_imager MODULE ${testing_imager_SRCS} )
target_link_libraries(testing_imager png drivers)

