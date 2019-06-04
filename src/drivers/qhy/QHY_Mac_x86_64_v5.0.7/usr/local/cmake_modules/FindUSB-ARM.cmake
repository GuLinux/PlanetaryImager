#===========================================================================
# Title      : CMakeLists.txt
# Note       : This cmake generates libusb lirary for ARM OS.
# Written by : Jan Soldan
# Version    : 1.0
# Date       : 6/2017
# NOTE       : Based on work of Andreas Schneider <mail@cynapses.org>
#===========================================================================
# - Try to find libusb-1.0
# Once done this will define
#
#  LIBUSB_ARM_FOUND - system has libusb
#  LIBUSB_ARM_INCLUDE_DIRS - the libusb include directory
#  LIBUSB_ARM_LIBRARIES - Link these to use libusb
#  LIBUSB_ARM_DEFINITIONS - Compiler switches required for using libusb
#
#  Adapted from cmake-modules Google Code project
#
#  Copyright (c) 2006 Andreas Schneider <mail@cynapses.org>
#
#  (Changes for libusb) Copyright (c) 2008 Kyle Machulis <kyle@nonpolynomial.com>
#
# Redistribution and use is allowed according to the terms of the New BSD license.
#
# CMake-Modules Project New BSD License
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
#
# * Neither the name of the CMake-Modules Project nor the names of its
#   contributors may be used to endorse or promote products derived from this
#   software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

if (LIBUSB_ARM_LIBRARIES AND LIBUSB_ARM_INCLUDE_DIRS)

    # in cache already
    set(LIBUSB_FOUND TRUE)

else (LIBUSB_ARM_LIBRARIES AND LIBUSB_ARM_INCLUDE_DIRS)

    if (${PLATFORM} MATCHES "rpi2-gcc-4.9")

        find_path (LIBUSB_ARM_INCLUDE_DIR
            NAMES libusb.h
            PATHS ${WORKSPACE_ROOT}/x-tools/armv7-rpi2-linux-gnueabihf-gcc-4.9/libusb/include
            PATH_SUFFIXES libusb-1.0
        )

        find_library (LIBUSB_ARM_LIBRARY
            NAMES usb-1.0
            PATHS ${WORKSPACE_ROOT}/x-tools/armv7-rpi2-linux-gnueabihf-gcc-4.9/libusb/lib
        )

    elseif (${PLATFORM} MATCHES "rpi2-gcc-7.1")

        find_path (LIBUSB_ARM_INCLUDE_DIR
            NAMES libusb.h
            PATHS ${WORKSPACE_ROOT}/x-tools/armv7-rpi2-linux-gnueabihf-gcc-7.1/libusb/include
            PATH_SUFFIXES libusb-1.0
        )

        find_library (LIBUSB_ARM_LIBRARY
            NAMES usb-1.0
            PATHS ${WORKSPACE_ROOT}/x-tools/armv7-rpi2-linux-gnueabihf-gcc-7.1/libusb/lib
        )

    elseif (${PLATFORM} MATCHES "rpi3-gcc-4.9")

        find_path (LIBUSB_ARM_INCLUDE_DIR
            NAMES libusb.h
            PATHS ${WORKSPACE_ROOT}/x-tools/armv8-rpi3-linux-gnueabihf-gcc-4.9/libusb/include
            PATH_SUFFIXES libusb-1.0
        )

        find_library (LIBUSB_ARM_LIBRARY
            NAMES usb-1.0
            PATHS ${WORKSPACE_ROOT}/x-tools/armv8-rpi3-linux-gnueabihf-gcc-4.9/libusb/lib
        )

    elseif (${PLATFORM} MATCHES "rpi3-gcc-7.1")

        find_path (LIBUSB_ARM_INCLUDE_DIR
            NAMES libusb.h
            PATHS ${WORKSPACE_ROOT}/x-tools/armv8-rpi3-linux-gnueabihf-gcc-7.1/libusb/include
            PATH_SUFFIXES libusb-1.0
        )

        find_library (LIBUSB_ARM_LIBRARY
            NAMES usb-1.0
            PATHS ${WORKSPACE_ROOT}/x-tools/armv8-rpi3-linux-gnueabihf-gcc-7.1/libusb/lib
        )

    endif() # PLATFORM

    set (LIBUSB_ARM_INCLUDE_DIRS
        ${LIBUSB_ARM_INCLUDE_DIR}
    )

    set (LIBUSB_ARM_LIBRARIES
        ${LIBUSB_ARM_LIBRARY}
    )

    if (LIBUSB_ARM_INCLUDE_DIRS AND LIBUSB_ARM_LIBRARIES)
        set(LIBUSB_ARM_FOUND TRUE)
    endif (LIBUSB_ARM_INCLUDE_DIRS AND LIBUSB_ARM_LIBRARIES)

    if (LIBUSB_ARM_FOUND)
        if (NOT USB-ARM_FIND_QUIETLY)
            message (STATUS "Found libusb-1.0:")
            message (STATUS " - Includes: ${LIBUSB_ARM_INCLUDE_DIRS}")
            message (STATUS " - Libraries: ${LIBUSB_ARM_LIBRARIES}")
        endif (NOT USB-ARM_FIND_QUIETLY)
    
        set (CMAKE_REQUIRED_INCLUDES ${LIBUSB_ARM_INCLUDE_DIRS})
        set (CMAKE_REQUIRED_LIBRARIES ${LIBUSB_ARM_LIBRARIES})
        include (CheckCXXSourceCompiles)
        check_cxx_source_compiles("#include <libusb-1.0/libusb.h> 
        int main() { libusb_error_name(0); return 0; }" ERROR_NAME_COMPILE)
    
        if (NOT ERROR_NAME_COMPILE)
            add_definitions ("-DNO_ERROR_NAME")
            message (STATUS " - 1.0.8 or older")
        endif (NOT ERROR_NAME_COMPILE)
    
    else (LIBUSB_ARM_FOUND)

        if (USB-ARM_FIND_REQUIRED)
            message (FATAL_ERROR "Could not find libusb-1.0. Please install libusb-1.0 along with the development package.")
        endif (USB-ARM_FIND_REQUIRED)
  
    endif (LIBUSB_ARM_FOUND)

    # show the LIBUSB_ARM_INCLUDE_DIRS and LIBUSB_ARM_LIBRARIES variables only in the advanced view
    mark_as_advanced (LIBUSB_ARM_INCLUDE_DIRS LIBUSB_ARM_LIBRARIES)

endif (LIBUSB_ARM_LIBRARIES AND LIBUSB_ARM_INCLUDE_DIRS)
