/*
 * Copyright (C) 2016  Marco Gulino <marco@gulinux.net>
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

#include "supporteddrivers.h"
#include <QLibrary>
#include <QDebug>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QDirIterator>
#include <QJsonDocument>
#include "drivers/imagerexception.h"

using namespace std;

struct SupportedDriver {
  typedef shared_ptr<SupportedDriver> ptr;
  SupportedDriver(const QString &name, const QVariantMap &info, const shared_ptr<QLibrary> &library) : name(name), info(info), library(library) {}
    QString name;
    QVariantMap info;
    shared_ptr<QLibrary> library;
    shared_ptr<Driver> _driver;
    shared_ptr<Driver> driver();
};

typedef int (*LoadDriverFunctionReady)();


std::shared_ptr<Driver> SupportedDriver::driver()
{
  if(!_driver) {
    qDebug() << "Initializing driver" << library->fileName();
    try {
      Driver *driverptr = ((LoadDriverFunction) library->resolve(PLANETARY_IMAGER_DRIVER_LOAD_F))();
      qDebug() << "Loaded driver address: " << (uint64_t)driverptr;
      _driver = shared_ptr<Driver>(driverptr);
    } catch(const Imager::exception &e) {
      qWarning() << "Error loading driver: " << e.what();
    }
  }
  qDebug() << "Driver loaded: " << !!_driver;
  return _driver;
}



DPTR_IMPL(SupportedDrivers) {
  SupportedDrivers *q;
  QList<SupportedDriver::ptr> supported_drivers;

  void find_drivers(const QString &directory);
  void load_driver(const QString &filename);
};


SupportedDrivers::SupportedDrivers(const QStringList &driversPath) : dptr(this)
{
  for(const QString &path: driversPath)
    d->find_drivers(path);
}

void SupportedDrivers::Private::find_drivers(const QString& directory)
{
  qDebug() << "Looking for drivers in " << directory;
  if(QFileInfo{directory}.isDir()) {
    QDirIterator it{directory, QDirIterator::Subdirectories | QDirIterator::FollowSymlinks};
    while(it.hasNext()) {
      load_driver(it.next());
    }
  }
}


void SupportedDrivers::Private::load_driver(const QString& filename)
{
  if(!filename.startsWith("driver_") && ! filename.endsWith(".json")) {
    return;
  }
  QString library_name = filename;
  library_name.remove(".json");

  qDebug() << "[??] trying " << filename << ": " << library_name;

  auto driver = make_shared<QLibrary>(library_name);
  if(driver->load()) {
    qDebug() << "[OK] Driver " << library_name << " loaded successfully";
    if(driver->resolve(PLANETARY_IMAGER_DRIVER_LOAD_F)) {
        QFile json_file(filename);
        json_file.open(QIODevice::ReadOnly);
        QVariantMap driver_info = QJsonDocument::fromJson(json_file.readAll()).toVariant().toMap();
        
        qDebug() << "[OK] PlanetaryImager_loadDriver resolved on " << library_name;
        supported_drivers.push_back(make_shared<SupportedDriver>(
          QFileInfo(library_name).baseName(),
          driver_info,
          driver
        ));
    } else {
        qWarning() << "[ERR] Error resolving PlanetaryImager_loadDriver function on " << library_name << ":" << driver->errorString();
    }
  } else {
    qWarning() << "[ERR] Error loading driver" << library_name << ":" << driver->errorString();
  }
}


SupportedDrivers::~SupportedDrivers()
{
}

void SupportedDrivers::aboutToQuit()
{
  for(auto supported_driver: d->supported_drivers)
    if(supported_driver->driver()) {
      supported_driver->driver()->aboutToQuit();
    } 
}


Driver::Cameras SupportedDrivers::cameras() const
{
  qDebug() << "Detecting active cameras";
  Cameras cameras;
  for(auto supported_driver: d->supported_drivers) {
    qDebug() << "Checking cameras on driver " << supported_driver->name;
    if(supported_driver->driver()) {
      auto driver_cameras = supported_driver->driver()->cameras();
      qDebug() << "Found" << driver_cameras.size() << "on driver" << supported_driver->name;
      cameras.append(driver_cameras);
    } else {
      qWarning() << "Driver" << supported_driver->name << "doesn't seem to be correctly loaded";
    }
  }
  qDebug() << "Detected cameras: " << cameras.size();
  return cameras;
}


