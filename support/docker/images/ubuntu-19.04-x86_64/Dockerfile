FROM ubuntu:19.04
COPY qemu-arm-static /usr/bin
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    autoconf \
    qtbase5-dev \
    qtdeclarative5-dev \
    build-essential \
    cmake \
    libopencv-dev \
    libboost-all-dev \
    git \
    libudev-dev \
    libccfits-dev \
    && apt-get clean

COPY configuration-ubuntu-19.04.cmake /
COPY debian-package-checker.sh /usr/local/bin/
ENV CMAKE_EXTRA_OPTIONS -DCMAKE_INCLUDE_EXT_FILE=/configuration-ubuntu-19.04.cmake -DCPACK_GENERATOR=DEB $CMAKE_EXTRA_OPTIONS
ENV TEST_PACKAGE_CMD="/usr/local/bin/debian-package-checker.sh"
RUN cd / && git clone https://github.com/libusb/libusb.git libusb-git && cd libusb-git && ./autogen.sh --enable-static=yes --enable-shared=no --with-pic --prefix=/opt/libusb-git $LIBUSB_CONFIGURE_OPTS && make all install $LIBUSB_MAKE_OPTS && cd / && rm -rf /libusb-git
ENV CMAKE_PREFIX_PATH="/opt/libusb-git;$CMAKE_PREFIX_PATH"
WORKDIR /build

VOLUME /dest
COPY entrypoint /entrypoint
ENTRYPOINT ["bash", "/entrypoint"]

