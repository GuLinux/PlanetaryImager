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

#ifndef V4L2FORMATS_H
#define V4L2FORMATS_H

#include "c++/dptr.h"
#include <QSize>
#include <QList>
#include <linux/videodev2.h>
#include "commons/fwd.h"

FWD_PTR(V4L2Device)

FWD_PTR(V4L2Formats)
FWD_PTR(V4L2Format)
FWD_PTR(V4L2Resolution)

class V4L2Formats
{
public:
  V4L2Formats(const V4L2DevicePtr &device);
  ~V4L2Formats();
  QList<V4L2FormatPtr> formats() const;
  V4L2ResolutionPtr current_resolution() const;
  v4l2_format current_v4l2_format() const;
private:
  DPTR
};

class V4L2Resolution {
public:
  V4L2Resolution(const v4l2_frmsizeenum &frmsizeenum, const V4L2DevicePtr &device, V4L2Format &format);
  ~V4L2Resolution();
  QSize size() const;
  std::size_t area() const;
  V4L2Format &format();
  void set();
  uint32_t index() const;
  bool operator==(const V4L2Resolution &other) const;
private:
  DPTR
};

class V4L2Format {
public:
  V4L2Format(const v4l2_fmtdesc &fmtdesc, const V4L2DevicePtr &device);
  ~V4L2Format();
  QString name() const;
  QString description() const;
  QList<V4L2ResolutionPtr> resolutions() const;
  V4L2ResolutionPtr max_resolution() const;
  void set(V4L2Resolution *resolution = nullptr);
  bool operator==(const V4L2Format &other) const;
  uint32_t fourcc() const;
private:
  DPTR
};

#endif // V4L2FORMATS_H
