set(qhy_SOURCES qhydriver.cpp qhyccdimager.cpp)
configure_file(qhy_messages.in qhy_messages.cpp)
add_library(qhy_driver STATIC ${qhy_SOURCES})
