FROM ubuntu:15.04
MAINTAINER Marco Gulino <marco@gulinux.net>
RUN apt-get update && apt-get install -y build-essential cmake git bash curl "^libxcb.*" libx11-xcb-dev libglu1-mesa-dev libxrender-dev libxi-dev
ENV QT5_MAJOR=5
ENV QT5_MINOR=5
ENV QT5_PATCH=0

RUN curl -L http://download.qt.io/official_releases/qt/${QT5_MAJOR}.${QT5_MINOR}/${QT5_MAJOR}.${QT5_MINOR}.${QT5_PATCH}/submodules/qtbase-opensource-src-${QT5_MAJOR}.${QT5_MINOR}.${QT5_PATCH}.tar.xz \
    -O
RUN tar xf /qtbase-opensource-src-${QT5_MAJOR}.${QT5_MINOR}.${QT5_PATCH}.tar.xz
WORKDIR /qtbase-opensource-src-${QT5_MAJOR}.${QT5_MINOR}.${QT5_PATCH}
RUN ./configure -prefix /opt/qt-5.5 -opensource -release -c++11 -opensource -confirm-license -no-accessibility \
 -qt-xcb -qt-xkbcommon -no-alsa -no-pulseaudio -no-gtkstyle -no-reduce-relocations -no-cups && make -j4 && make install

RUN apt-get install -y pkg-config libusb-1.0-0-dev rsync
RUN rm -rf /qtbase*

RUN mkdir /PlanetaryImager
RUN mkdir /PlanetaryImager-dist
RUN mkdir /PlanetaryImager-install
VOLUME "/PlanetaryImager"
VOLUME "/PlanetaryImager-dist"
WORKDIR /PlanetaryImager-build

RUN find /opt/ \( -name "*.so*" -or -name "DejaVu*ttf" \) -not -regex '.*examples.*' | rsync -avPh --files-from=- / /PlanetaryImager-install/
RUN mkdir -p /PlanetaryImager-install/usr/bin
COPY planetary_imager_launcher /PlanetaryImager-install/usr/bin/

CMD cmake /PlanetaryImager -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release -DQt5Widgets_DIR=/opt/qt-5.5/lib/cmake/Qt5Widgets -DQt5Test_DIR=/opt/qt-5.5/lib/cmake/Qt5Test/ \
  && make -j4 && make install DESTDIR=/PlanetaryImager-install && cd /PlanetaryImager-install && tar czf /PlanetaryImager-dist/PlanetaryImager.tar.gz .