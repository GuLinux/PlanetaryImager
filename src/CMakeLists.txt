set(QHYImager_SRCS QHYImager.cpp main.cpp)

add_executable(qhyimager ${QHYImager_SRCS})
target_link_libraries(qhyimager Qt5::Widgets)

install(TARGETS qhyimager RUNTIME DESTINATION bin)
