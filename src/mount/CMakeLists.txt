set(mount-SRCS mount.cpp)
add_backend_dependencies(mount)

if(HAVE_LIBINDI)
    set(enabled_mounts ${enabled_mounts} INDI CACHE INTERNAL "")
    set(mount-SRCS ${mount-SRCS} indi.cpp)

    # Do not use ${LIBINDI_LIBRARIES}; on some installations it may be missing "libindiclient" (which we need),
    # and contain "libindidriver" (not needed, and requiring additional dependencies).
    find_package(Threads REQUIRED)

    add_backend_dependencies(indiclient ${CMAKE_THREAD_LIBS_INIT})
    message("INDI support enabled (Mount).")
else()
    message("INDI support disabled: libindi not found.")
endif()

add_library(mount STATIC ${mount-SRCS})

