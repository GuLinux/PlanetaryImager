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

#include "v4l2device.h"
#include "c++/dptr.h"
#include <QSize>
#include <QList>
#include <linux/videodev2.h>


class V4L2Formats
{
public:
  typedef std::shared_ptr<V4L2Formats> ptr;
  V4L2Formats(const V4L2Device::ptr &device);
  ~V4L2Formats();
  class Format;
  class Resolution;
  QList<std::shared_ptr<Format>> formats() const;
  std::shared_ptr<Resolution> current_resolution() const;
  v4l2_format current_v4l2_format() const;
private:
  DPTR
};

class V4L2Formats::Resolution {
public:
  Resolution(const v4l2_frmsizeenum &frmsizeenum, const V4L2Device::ptr &device, Format &format);
  ~Resolution();
  typedef std::shared_ptr<Resolution> ptr;
  QSize size() const;
  std::size_t area() const;
  Format &format();
  void set();
  uint32_t index() const;
  bool operator==(const Resolution &other) const;
private:
  DPTR
};

class V4L2Formats::Format {
public:
  Format(const v4l2_fmtdesc &fmtdesc, const V4L2Device::ptr &device);
  ~Format();
  typedef std::shared_ptr<Format> ptr;
  QString name() const;
  QString description() const;
  QList<Resolution::ptr> resolutions() const;
  Resolution::ptr max_resolution() const;
  void set(Resolution *resolution = nullptr);
  bool operator==(const Format &other) const;
  uint32_t fourcc() const;
private:
  DPTR
};

#endif // V4L2FORMATS_H
