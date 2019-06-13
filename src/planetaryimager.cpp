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
#include "Qt/qt_functional.h"
#include "commons/messageslogger.h"
#include <QThread>
#include <QTimer>
#include "image_handlers/saveimages.h"
#include "drivers/driver.h"

#ifdef STATIC_WINDOWS_PLUGIN
#pragma message("Initializing Qt static plugins")
#include <QtPlugin>
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
#endif

DPTR_IMPL(PlanetaryImager) {
  DriverPtr driver;
  ImageHandlerPtr imageHandler;
  SaveImagesPtr saveImages;
  Configuration &configuration;
  PlanetaryImager *q;
  QList<CameraPtr> cameras;
  Imager *imager = nullptr;

  void initDevicesWatcher();
};

PlanetaryImager::PlanetaryImager(
  const DriverPtr &driver,
  const ImageHandlerPtr &imageHandler,
  const SaveImagesPtr &saveImages,
  Configuration &configuration
) : QObject{}, dptr(driver, imageHandler, saveImages, configuration, this)
{
  QThreadPool::globalInstance()->setMaxThreadCount(std::max(5, QThread::idealThreadCount()));
  d->initDevicesWatcher();
}

PlanetaryImager::~PlanetaryImager()
{
}

Imager * PlanetaryImager::imager() const
{
  return d->imager;
}

SaveImagesPtr PlanetaryImager::saveImages() const
{
  return d->saveImages;
}

Configuration &PlanetaryImager::configuration() const
{
  return d->configuration;
}


QList<CameraPtr> PlanetaryImager::cameras() const
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
  GuLinux::qAsyncR<QList<CameraPtr>>([this] { return d->driver->cameras(); }, [this](const QList<CameraPtr> &cameras) {
    d->cameras = cameras;
    emit camerasChanged();
  }, this);
}

void PlanetaryImager::open(const CameraPtr& camera)
{
  if(d->imager)
    d->imager->destroy();
  auto openImager = [this, camera] () -> Imager *{
    try {
      auto imager = camera->imager(d->imageHandler);
      imager->setCaptureEndianess(d->configuration.capture_endianess());
      imager->moveToThread(this->thread());
      imager->setParent(this);
      return imager;
    } catch(const std::exception &e) {
      MessagesLogger::queue(MessagesLogger::Error, tr("Initialization Error"), tr("Error initializing imager %1: \n%2") % camera->name() % e.what());
      return nullptr;
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

  static QStringList entries = QDir(usbfsdir).entryList();

  connect(notifyTimer, &QTimer::timeout, [=]{
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

