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

#ifndef ASIIMAGINGWORKER_H
#define ASIIMAGINGWORKER_H

#include "ASICamera2.h"
#include <vector>
#include "drivers/imager.h"
#include "drivers/imagerthread.h"
#include <QRect>

class ASIImagingWorker : public ImagerThread::Worker {
public:
  typedef std::shared_ptr<ASIImagingWorker> ptr;
  ASIImagingWorker(const QRect &roi, int bin, const ASI_CAMERA_INFO &info, ASI_IMG_TYPE format);
  ~ASIImagingWorker();
  Frame::ptr shoot() override;

  QRect roi() const;
  ASI_IMG_TYPE format() const;
  int bin() const;
private:
  DPTR
};

#endif // ASIIMAGINGWORKER_H
