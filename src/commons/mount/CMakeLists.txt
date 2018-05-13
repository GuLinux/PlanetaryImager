pkg_check_modules(LIBINDI libindi)
if(LIBINDI_FOUND)
    set(enabled_mounts ${enabled_mounts} INDI CACHE INTERNAL "")

    set(mount-SRCS ${CMAKE_CURRENT_SOURCE_DIR}/mount.cpp)

    list(APPEND planetaryimager-commons-SRCS ${mount-SRCS})
    update_parent(planetaryimager-commons-SRCS)
    
    list(APPEND planetaryimager-commons-INCLUDE_DIRS ${LIBINDI_INCLUDE_DIRS})
    update_parent(planetaryimager-commons-INCLUDE_DIRS)
    
    list(APPEND planetaryimager-commons-LINK-DIRS ${LIBINDI_LIBRARY_DIRS})
    update_parent(planetaryimager-commons-LINK-DIRS)
    # Do not use ${LIBINDI_LIBRARIES}; on some installations it may be missing "libindiclient" (which we need),
    # and contain "libindidriver" (not needed, and requiring additional dependencies).
    list(APPEND planetaryimager-commons-LIBS indiclient)
    find_package(Threads REQUIRED)
    list(APPEND planetaryimager-commons-LIBS ${CMAKE_THREAD_LIBS_INIT})
    update_parent(planetaryimager-commons-LIBS)
    
    list(APPEND planetaryimager-commons-DEFS -DINDI_MOUNT_SUPPORTED=1)
    update_parent(planetaryimager-commons-DEFS)
    
    message("INDI support enabled.")
else()
    list(APPEND planetaryimager-commons-DEFS -DINDI_MOUNT_SUPPORTED=0)
    message("INDI support disabled: libindi not found.")
endif()
