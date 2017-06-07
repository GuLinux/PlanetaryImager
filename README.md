# PlanetaryImager
=================

Qt capture software for astronomy, mainly planetary shooting.

Useful Links
------------

 * [Project Homepage - Full description, screenshots](http://blog.gulinux.net/en/planetary-imager)
 * [Stable Releases and Downloads](https://github.com/GuLinux/PlanetaryImager/releases)
 * [Development releases](https://www.dropbox.com/sh/7ofjp0s6gbelff4/AAAmNhLU4sDKuTKPojSxA-P7a/development?dl=0&lst=)
 * [Report a bug](https://github.com/GuLinux/PlanetaryImager/issues)

Supported Cameras
-----------------

Mainly ZWO ASI Cameras (my camera is an ASI178mm), v4l2 (including DMK and Celestron Skyris), QHY, IIDC (including all FLIR/Point Grey FireWire and USB 2/3 models).


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

Source download
---------------

You need to use the following commands to clone both planetary imager and its submodules.
A simple git clone won't work!

Master (development) version:

    git clone --recursive https://github.com/GuLinux/PlanetaryImager.git 

Stable release (replace 'v0.6.2' with the current stable release, if needed):

    git clone --recursive -b v0.6.2 https://github.com/GuLinux/PlanetaryImager.git 
    

Compile Howto
-------------

These are generic instructions, and assume you're already familiar with building a package from sources.
If not, please try binary releases first.

    mkdir build
    cd build
    cmake .. -DCMAKE_INSTALL_PREFIX=/usr
    make all && sudo make install

To enable a driver not enabled by default, either add `DEFAULT_ON` as argument to `add_driver()` in driver's CMakeLists.txt file, or set `BUILD_DRIVER_xxx:BOOL=ON` in CMakeCache.txt in the build directory.

To run without installing, you must specify the drivers location as a parameter. E.g. when in build directory, use:


    src/planetary_imager --drivers .


Uninstall
---------

If you installed PlanetaryImager from a precompiled package, just use your package manager to remove it.

If you installed PlanetaryImager from sources, just go to the source directory where you compiled it, and run the folowing:
```cat install_manifest.txt  | while read file; do [[ -r $file ]] && ! [[ -d $file ]] && sudo rm -f $file; done```

When compiling from sources, it is often advised to uninstall the previous version before installing the new one.


Windows version
---------------

Currently, windows snapshots are compiled using [MXE](http://mxe.cc/).
It has not been really tested a lot, but to help investigating problems, please try another Windows application first, to see if your camera driver is properly working.

OSX Version
-----------

I have been unable to create a binary package for OSX, since I don't really have a development environment on OSX, but I could do a few tests, and I managed to compile it by using [homebrew](https://brew.sh/)

To get a working version, please follow these instructions:

 * Install homebrew, following instructions on their website
 * Run the following command to install dependencies:
 
    ```brew install git cmake qt5 cfitsio ccfits opencv3 boost```
 * clone the repository (follow the instruction in the "Source download" section)
 * compile (follow instructions in "Compile Howto")
   * When running cmake, be sure to add the following directive (change Qt version if needed)
   
     ```-DCMAKE_PREFIX_PATH=/usr/local/Cellar/qt5/5.8.0_1```
 * run `planetary_imager` from your command line
 

IIDC (libdc1394)
----------------

If you camera is not detected, you may need to use a newer version of libdc1394. In case of building the library on your own, the supported USB device ids are hard-coded in usb/control.c in `usb_products[]`. You can check your camera's VID/PID by calling `lsusb`, e.g. (for the Firefly MV camera):

    ...
    Bus 002 Device 003: ID 1e10:2001 Point Grey Research, Inc.
    ...

 
Credits
-------

Application Icon: [Hat Saturn](https://www.iconfinder.com/icons/37878/hat_planet_saturn_icon) by [Denis Sazhin](http://iconka.com/)
Toolbar/action icons: [TWG Retina Display Icons](http://blog.twg.ca/2010/11/retina-display-icon-set/), darkened and with a few customizations to add more actions.
