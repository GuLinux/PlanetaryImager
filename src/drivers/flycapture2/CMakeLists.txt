if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(FC2_INCLUDE_DIR "C:\\Program Files\\Point Grey Research\\FlyCapture2\\include")
  set(FC2_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR})  # The import library is created during build, in current directory
  set(FC2_LIB FlyCapture2_C)
else()
  set(FC2_INCLUDE_DIR /usr/include/flycapture)
  set(FC2_LIB_DIR /usr/lib)
  set(FC2_LIB flycapture flycapture-c)
endif()

file(GLOB fc2_SRCS *.cpp)

include_directories(${FC2_INCLUDE_DIR})
link_directories(${FC2_LIB_DIR})

add_driver(NAME flycapture2 OS Linux Windows Darwin DEFAULT_ON SRCS ${fc2_SRCS} LINK ${FC2_LIB})


if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_custom_command(
        TARGET flycapture2 PRE_LINK
        COMMAND dlltool -d ${CMAKE_CURRENT_SOURCE_DIR}/FlyCapture2_C.def -l ${CMAKE_CURRENT_BINARY_DIR}/libFlyCapture2_C.a
    )
endif()
