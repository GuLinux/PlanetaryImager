if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(FC2_INCLUDE_DIR "C:\\Program Files\\Point Grey Research\\FlyCapture2\\include")
  set(FC2_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR})
  set(FC2_LIB FlyCapture2_C)
else()
  set(FC2_INCLUDE_DIR /usr/include/flycapture)
  set(FC2_LIB_DIR /usr/lib)
  set(FC2_LIB flycapture-c)
endif()

file(GLOB fc2_SRCS *.cpp)

include_directories(${FC2_INCLUDE_DIR})
link_directories(${FC2_LIB_DIR})

add_driver(NAME flycapture2 OS Linux Windows Darwin DEFAULT_ON SRCS ${fc2_SRCS} LINK ${FC2_LIB})
