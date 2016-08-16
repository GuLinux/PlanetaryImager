/*
 * Copyright (C) 2016 Marco Gulino (marco AT gulinux.net)
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 */

#ifndef ASIIMAGINGWORKER_H
#define ASIIMAGINGWORKER_H

#include "ASICamera2.h"
#include <vector>
#include "drivers/imager.h"
#include "drivers/imagerthread.h"
#include <QRect>

class ASIImagingWorker : public ImagerThread::Worker {
public:
  ASIImagingWorker(const QRect &roi, int bin, const ASI_CAMERA_INFO &info, ASI_IMG_TYPE format);
  ~ASIImagingWorker();
  virtual bool shoot(const ImageHandlerPtr& imageHandler);
  virtual void start();
  virtual void stop();

  QRect roi() const;
  ASI_IMG_TYPE format() const;
  int bin() const;
private:
  DPTR
};

#endif // ASIIMAGINGWORKER_H
