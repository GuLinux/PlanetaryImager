file(GLOB iidc_SRCS *.cpp)

add_driver(NAME iidc OS Linux Darwin DEFAULT_ON SRCS ${iidc_SRCS})
