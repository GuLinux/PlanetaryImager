FROM ubuntu:latest
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    autopoint autoconf automake bash bison bzip2 flex \
    libgdk-pixbuf2.0-dev gettext git gperf intltool make \
    sed libffi-dev libtool p7zip-full patch lzip \
    perl python ruby scons unzip wget xz-utils libtool-bin \
    && apt-get clean
RUN cd / && git clone https://github.com/mxe/mxe.git && cd /mxe && \
    make MXE_TARGETS='x86_64-w64-mingw32.shared' MXE_PLUGIN_DIRS='/mxe/plugins/gcc5' \
        nsis ccfits boost cmake qtbase qtdeclarative \
        ffmpeg jpeg libpng tiff opencv \ 
    	clean-junk 

ENV PATH=/mxe/usr/bin:$PATH


RUN cd /tmp && wget --quiet http://heasarc.gsfc.nasa.gov/FTP/software/fitsio/c/cfitsio3410.tar.gz && \
    wget --quiet https://heasarc.gsfc.nasa.gov/fitsio/CCfits/CCfits-2.5.tar.gz && \
    tar xf cfitsio3410.tar.gz && tar xf CCfits-2.5.tar.gz && \ 
    cd /tmp/cfitsio && ./configure --host=x86_64-w64-mingw32.shared --prefix=/mxe/usr/x86_64-w64-mingw32.shared FC=${MXE_PREFIX}-gfortran && make -j$MXE_PARALLEL_CPU_JOBS && make install && \
    cd /tmp/CCfits && ./configure  --host=x86_64-w64-mingw32.shared --prefix=/mxe/usr/x86_64-w64-mingw32.shared --build=static --enable-static --disable-shared --with-cfitsio=/mxe/usr/x86_64-w64-mingw32.shared && \
    make && make install && cd / && rm -rf /tmp/*
#RUN ln -s /mxe/usr/bin/x86_64-w64-mingw32.shared-makensis /usr/bin/makensis

ENV CMAKE_BIN=x86_64-w64-mingw32.shared-cmake

COPY windows-deps-x86_64.cmake /
ENV CMAKE_EXTRA_OPTIONS -DCMAKE_INCLUDE_POST_FILE=/windows-deps-x86_64.cmake -Dcflags_fpic=Off -DCMAKE_INSTALL_PREFIX=/ -DCPACK_GENERATOR=ZIP

WORKDIR /build

VOLUME /dest
COPY entrypoint /entrypoint
ENTRYPOINT ["bash", "/entrypoint"]

