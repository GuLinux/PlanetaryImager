file(GLOB testing_imager_SRCS *.cpp *.qrc)
add_driver(NAME simulator OS Linux Windows Darwin SRCS ${testing_imager_SRCS} LINK png)
