# PlanetaryImager
=================

Qt capture software for astronomy, mainly planetary shooting 

Supported Cameras
-----------------
Currently this application supports only QHY based cameras, mainly QHY5IIm-L (mine).

Requirements
------------
 * Qt version 5.5
 * A recent boost library version (tested with 1.55 and above).


Source download
---------------

Remember to clone the submodule repository, or it won't compile!
  

    git clone https://github.com/GuLinux/PlanetaryImager.git
    cd PlanetaryImager
    git submodule init
    git submodule update
    

Compile Howto
-------------

    mkdir build
    cmake .. -DCMAKE_INSTALL_PREFIX=/usr
    make all && sudo make install
    
