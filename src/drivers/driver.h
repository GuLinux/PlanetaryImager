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
#ifndef PLANETARY_IMAGER_DRIVER_H
#define PLANETARY_IMAGER_DRIVER_H
#include <QList>
#include <QString>
#include <QtPlugin>
#include "imager.h"
#include "dptr.h"

class Driver : public QObject {
  Q_OBJECT
public:
  typedef std::shared_ptr<Driver> ptr;
  class Camera {
  public:
    typedef std::shared_ptr<Camera> ptr;
    virtual QString name() const = 0;
    virtual Imager *imager(const ImageHandler::ptr &imageHandler) const = 0;
  };
  typedef QList<Camera::ptr> Cameras; 
  virtual void aboutToQuit();
  virtual Cameras cameras() const = 0;
};
typedef QList<Driver::ptr> Drivers;
typedef Driver *(*LoadDriverFunction)();

#ifdef Q_OS_WIN
#define EXPORT_DECL __declspec(dllexport)
#else
#define EXPORT_DECL
#endif

#define PLANETARY_IMAGER_DRIVER_LOAD_F "PlanetaryImager_loadDriver"
#define DECLARE_DRIVER_PLUGIN_INIT(DriverClass) extern "C" EXPORT_DECL Driver *PlanetaryImager_loadDriver() { return new DriverClass(); }

#endif
