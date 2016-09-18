/*
 * GuLinux Planetary Imager - https://github.com/GuLinux/PlanetaryImager
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

#ifndef QHYIMAGINGWORKER_H
#define QHYIMAGINGWORKER_H

#include "drivers/imagerthread.h"
#include "c++/dptr.h"

#include "qhyccd.h"

class QHYImagingWorker : public ImagerThread::Worker
{
public:
  QHYImagingWorker(qhyccd_handle *handle);
  ~QHYImagingWorker();
  virtual Frame::ptr shoot();
  virtual void start();
  virtual void stop();
private:
  DPTR
};

#endif // QHYIMAGINGWORKER_H
