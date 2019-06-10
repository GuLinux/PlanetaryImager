# PlanetaryImager
=================

Qt capture software for astronomy, mainly planetary shooting.


Useful Links
------------

 * [Project Homepage - Full description, screenshots](http://planetaryimager.gulinux.net)
 * [Releases and Downloads](https://github.com/GuLinux/PlanetaryImager/releases)
 * [Report a bug](https://github.com/GuLinux/PlanetaryImager/issues)

 
Supported Cameras
-----------------

 - ZWO ASI Cameras
 - V4L2 (including DMK and Celestron Skyris)
 - QHY
 - FLIR/Point Grey via FlyCapture2 (all interfaces)
 - IIDC (including all FLIR/Point Grey FireWire and USB models)


Requirements
------------
 * Qt version >= 5.5
 * A recent boost library version (tested with 1.55 and above).
 * c++14 compliant compiler (tested with gcc >= 5 and clang)
 * OpenCV
 * boost (libboost-all-dev)
 * fxload (to load firmware on QHY cameras)
 * ccfits
 * libdc1394 (for IIDC driver)
 * FlyCapture2 SDK (for FLIR/Point Grey cameras)

 
Compile Howto
-------------

These are generic instructions, and assume you're already familiar with building a package from sources.
If not, please try binary releases first.

    mkdir build
    cd build
    cmake .. -DCMAKE_INSTALL_PREFIX=/usr
    make all && sudo make install

To enable a driver not enabled by default, either add `DEFAULT_ON` as argument to `add_driver()` in driver's `CMakeLists.txt file`, or set `BUILD_DRIVER_xxx:BOOL=ON` in CMakeCache.txt in the build directory.

To run without installing, you must specify the drivers location as a parameter. E.g. when in build directory, use:


    src/planetary_imager --drivers .
    
Please look at the main website for more information on compiling and developing PlanetaryImager.


Uninstall
---------

If you installed PlanetaryImager from a precompiled package, just use your package manager to remove it.

If you installed PlanetaryImager from sources, just go to the source directory where you compiled it, and run the folowing:
```
cat install_manifest.txt  | while read file; do [[ -r $file ]] && ! [[ -d $file ]] && sudo rm -f $file; done
```

When compiling from sources, it is often advised to uninstall the previous version before installing the new one.


 
IIDC (libdc1394)
----------------

If you camera is not detected, you may need to use a newer version of `libdc1394`. In case of building the library on your own, the supported USB device ids are hard-coded in `usb/control.c` in `usb_products[]`. You can check your camera's VID/PID by calling `lsusb`, e.g. (for the Firefly MV camera):

    ...
    Bus 002 Device 003: ID 1e10:2001 Point Grey Research, Inc.
    ...
    
    
FlyCapture2 (FLIR/Point Grey)
-----------------------------

Installation of FlyCapture2 SDK is required (available from the FLIR website). On Windows, make sure to add the binaries directory to `PATH` (by default, `C:\Program Files\Point Grey Research\FlyCapture2\bin64` or `C:\Program Files (x86)\Point Grey Research\FlyCapture2\bin`). On Linux, if your distribution does not use `.deb` package format (which is used by the SDK), simply unpack the packages manually. In any case, run the install script to create `udev` device rules.

When building from sources, make sure that the `FC2_INCLUDE_DIR` variable in `src/drivers/flycapture2/CMakeLists.txt` is set correctly.

 
Credits
-------

Application Icon: [Moon Mosaic](https://www.iconfinder.com/icons/37878/hat_planet_saturn_icon) by [Marco Gulino](https://gulinux.net).
Toolbar/action icons: [TWG Retina Display Icons](http://blog.twg.ca/2010/11/retina-display-icon-set/), darkened and with a few customizations to add more actions.
IIDC & FlyCapture2 drivers: [Filip Szczerek](ga.software@yahoo.com) (GreatAttractor).
