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
#include "imager.h"
#include "dptr.h"

class Driver {
public:
  class Camera {
  public:
    typedef std::shared_ptr<Camera> ptr;
    virtual QString name() const = 0;
    virtual Imager *imager(const ImageHandler::ptr &imageHandler) const = 0;
  };
  typedef QList<Camera::ptr> Cameras; 
  
  virtual Cameras cameras() const = 0;
};
typedef std::shared_ptr<Driver> DriverPtr;

class SupportedDrivers : public Driver {
public:
  SupportedDrivers();
  ~SupportedDrivers();
  virtual Cameras cameras() const;
private:
  DPTR;
};

#endif
