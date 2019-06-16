# adapted from QHYCCD CMakeLists.txt
 IF (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "armv6l")
  set(PlanetaryImager_ARCH armv6)
 elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "armv7")
  set(PlanetaryImager_ARCH armv7)
 elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "aarch64|armv8")
  set(PlanetaryImager_ARCH armv8)
 elseif(CMAKE_SIZEOF_VOID_P EQUAL "8")
  set(PlanetaryImager_ARCH x86_64)
 else()
  set(PlanetaryImager_ARCH i686)
 endif (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "armv6l")

 message("Detected architecture: ${PlanetaryImager_ARCH}")
