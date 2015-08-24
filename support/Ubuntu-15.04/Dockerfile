FROM ubuntu:15.04
MAINTAINER Marco Gulino <marco@gulinux.net>

RUN apt-get update && apt-get install -y build-essential cmake git bash curl "^libxcb.*" libx11-xcb-dev libglu1-mesa-dev libxrender-dev libxi-dev qtbase5-dev qtmultimedia5-dev pkg-config libusb-1.0-0-dev rsync libboost-dev
ENV QT5_MAJOR=5
ENV QT5_MINOR=5
ENV QT5_PATCH=0

RUN mkdir /PlanetaryImager
RUN mkdir /PlanetaryImager-dist
RUN mkdir /PlanetaryImager-install
VOLUME "/PlanetaryImager"
VOLUME "/PlanetaryImager-dist"
WORKDIR /PlanetaryImager-build
RUN rm -rf /qtmultimedia*

CMD cmake /PlanetaryImager -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release && make -j4 && make install DESTDIR=/PlanetaryImager-install && cd /PlanetaryImager-install && tar czf /PlanetaryImager-dist/PlanetaryImager.tar.gz .