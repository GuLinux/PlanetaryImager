/*
 * GuLinux Planetary Imager - https://github.com/GuLinux/PlanetaryImager
 * Copyright (C) 2017  Marco Gulino <marco@gulinux.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "planetaryimager.h"
#include "Qt/functional.h"
#include "commons/messageslogger.h"
#include <QThread>
#include <QTimer>

DPTR_IMPL(PlanetaryImager) {
  Driver::ptr driver;
  ImageHandler::ptr imageHandler;
  SaveImages::ptr saveImages;
  Configuration &configuration;
  PlanetaryImager *q;
  
  Driver::Cameras cameras;
  Imager *imager = nullptr;
  
  void initDevicesWatcher();
};

PlanetaryImager::PlanetaryImager(
  const Driver::ptr &driver,
  const ImageHandler::ptr &imageHandler,
  const SaveImages::ptr &saveImages,
  Configuration &configuration
) : QObject{}, dptr(driver, imageHandler, saveImages, configuration, this)
{
  d->initDevicesWatcher();
}

PlanetaryImager::~PlanetaryImager()
{
}

Imager * PlanetaryImager::imager() const
{
  return d->imager;
}

SaveImages::ptr PlanetaryImager::saveImages() const
{
  return d->saveImages;
}

Configuration &PlanetaryImager::configuration() const
{
  return d->configuration;
}


Driver::Cameras PlanetaryImager::cameras() const
{
  return d->cameras;
}

void PlanetaryImager::startRecording()
{
  d->saveImages->startRecording(d->imager);
}

void PlanetaryImager::setRecordingPaused(bool paused)
{
  d->saveImages->setPaused(paused);
}

void PlanetaryImager::stopRecording()
{
  d->saveImages->endRecording();
}

void PlanetaryImager::scanCameras()
{
  GuLinux::qAsyncR<Driver::Cameras>([this] { return d->driver->cameras(); }, [this](const Driver::Cameras &cameras) {
    d->cameras = cameras;
    emit camerasChanged();
  }, this);
}

void PlanetaryImager::open(const Driver::Camera::ptr& camera)
{
  if(d->imager)
    d->imager->destroy();
  
  auto openImager = [this, camera] {
    try {
      auto imager = camera->imager(d->imageHandler);
      imager->moveToThread(this->thread());
      imager->setParent(this);
      return imager;
    } catch(const std::exception &e) {
      MessagesLogger::queue(MessagesLogger::Error, tr("Initialization Error"), tr("Error initializing imager %1: \n%2") % camera->name() % e.what());
    }
  };
  
  auto onImagerOpened = [this](Imager *imager) {
    d->imager = imager;
    if(imager) {
      connect(imager, &Imager::disconnected, this, &PlanetaryImager::cameraDisconnected);
      emit cameraConnected();
    }
  };
  GuLinux::qAsyncR<Imager *>(openImager, onImagerOpened, this);
}

void PlanetaryImager::closeImager()
{
  if(! d->imager)
    return;
  d->imager->destroy();
  d->imager->deleteLater();
  d->imager = nullptr;
}


void PlanetaryImager::Private::initDevicesWatcher()
{
  #ifdef Q_OS_LINUX
  auto notifyTimer = new QTimer(q);
  QString usbfsdir;
  for(auto path: QStringList{"/proc/bus/usb/devices", "/sys/bus/usb/devices"}) {
    if(QDir(path).exists())
      usbfsdir = path;
  }
  if(usbfsdir.isEmpty())
    return;
  connect(notifyTimer, &QTimer::timeout, [=]{
    static QStringList entries;
    auto current = QDir(usbfsdir).entryList();
    if(current != entries) {
      qDebug() << "usb devices changed";
      entries = current;
      q->scanCameras();
    }
  });
  notifyTimer->start(1500);
  #endif
}

void PlanetaryImager::quit()
{
  d->driver->aboutToQuit();
}

