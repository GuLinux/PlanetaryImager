FROM gulinux/planetaryimager_build:ubuntu_xenial_arm_base
RUN apt-get update && apt-get install -y autoconf libusb-1.0-0-dev libudev-dev qtbase5-dev build-essential cmake libopencv-dev libqcustomplot-dev libboost-all-dev git && apt-get clean
RUN cd / && git clone https://github.com/libusb/libusb.git libusb-git && cd libusb-git && ./autogen.sh && ./configure --enable-static=yes --enable-shared=no --prefix=/opt/libusb-git && make all install && cd / && rm -rf /libusb-git
WORKDIR /build

