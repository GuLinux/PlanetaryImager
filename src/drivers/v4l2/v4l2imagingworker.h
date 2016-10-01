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

#ifndef V4L2IMAGINGWORKER_H
#define V4L2IMAGINGWORKER_H
#include "drivers/imagerthread.h"
#include "c++/dptr.h"
#include "v4l2device.h"

struct v4l2_format;
class V4L2ImagingWorker : public ImagerThread::Worker
{
public:
  V4L2ImagingWorker(const V4L2Device::ptr &device, const v4l2_format &format);
  virtual ~V4L2ImagingWorker();
  Frame::ptr shoot() override;
private:
  DPTR
};

#endif // V4L2IMAGINGWORKER_H
