if(CMAKE_SYSTEM_NAME STREQUAL "Windows")

    #TODO: implement this
    
    #set(FC2_INCLUDE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/FC2CCD_Windows_SDK_x64_LatestVersion/x64/include)
    #set(FC2_DLL_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/FC2CCD_Windows_SDK_x64_LatestVersion/x64/dll)
else()
  set(FC2_INCLUDE_DIRECTORY /usr/include/flycapture)
  set(FC2_LIBRARY           flycapture)
endif()

file(GLOB fc2_SRCS *.cpp)

include_directories(${FC2_INCLUDE_DIRECTORY})

add_driver(NAME flycapture2 OS Linux Windows Darwin DEFAULT_ON SRCS ${fc2_SRCS} LINK ${FC2_LIBRARY})
