file(GLOB iidc_SRCS *.cpp)

add_driver(NAME iidc OS Linux Darwin DEFAULT_ON SRCS ${iidc_SRCS})

if(BUILD_DRIVER_iidc)
    pkg_check_modules(LIBDC1394 libdc1394-2 REQUIRED)
    find_library(IIDC_LIBRARIES ${LIBDC1394_LIBRARIES} HINTS ${LIBDC1394_LIBRARY_DIRS})
    include_directories(${LIBDC1394_INCLUDEDIR})
    target_link_libraries(iidc ${IIDC_LIBRARIES})
endif()
