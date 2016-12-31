# PlanetaryImager
=================

Qt capture software for astronomy, mainly planetary shooting.

Useful Links
------------

 * [Project Homepage - Full description, screenshots](http://blog.gulinux.net/en/planetary-imager)
 * [Releases and Downloads](https://github.com/GuLinux/PlanetaryImager/releases)
 * [Report a bug](https://github.com/GuLinux/PlanetaryImager/issues)

Supported Cameras
-----------------

Mainly ZWO ASI Cameras (my camera is an ASI178mm), v4l2 (including DMK and Celestron Skyris) and QHY


Requirements
------------
 * Qt version >= 5.5
 * A recent boost library version (tested with 1.55 and above).
 * c++14 compliant compiler (tested with gcc >= 5)
 * OpenCV
 * boost (libboost-all-dev)
 * fxload (to load firmware on QHY cameras)
 * ccfits

Source download
---------------

You need to use the following commands to clone both planetary imager and its submodules.
A simple git clone won't work!

Stable release (replace 'v0.6.2' with the current stable release, if needed):

    git clone --recursive -b v0.6.2 https://github.com/GuLinux/PlanetaryImager.git 

Master (development) version:

    git clone --recursive https://github.com/GuLinux/PlanetaryImager.git 
    

Compile Howto
-------------

    mkdir build
    cd build
    cmake .. -DCMAKE_INSTALL_PREFIX=/usr
    make all && sudo make install
    

Credits
-------

Icon: [Hat Saturn](https://www.iconfinder.com/icons/37878/hat_planet_saturn_icon) by [Denis Sazhin](http://iconka.com/)
