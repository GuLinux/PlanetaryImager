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

#ifndef FRAME_H
#define FRAME_H

#include <opencv2/opencv.hpp>
#include "c++/dptr.h"
#include <QSize>
#include <QDateTime>
#include <QVariantMap>
#include <chrono>
class Frame
{
public:
  typedef std::shared_ptr<Frame> ptr;
  using const_ptr = std::shared_ptr<const Frame>;
  enum ColorFormat {
      Mono,
      RGB,
      BGR,
      Bayer_RGGB,
      Bayer_GRBG,
      Bayer_GBRG,
      Bayer_BGGR,
    };
  enum ByteOrder { BigEndian, LittleEndian };
  Frame(ColorFormat colorFormat, const cv::Mat &image, ByteOrder byteOrder = BigEndian);
  Frame(uint8_t bpp, ColorFormat colorFormat, const QSize &resolution, ByteOrder byteOrder = BigEndian);
  ~Frame();
  std::size_t size() const;
  uint8_t *data();
  QSize resolution() const;
  cv::Mat mat() const;
  uint8_t channels() const;
  uint8_t bpp() const;
  QDateTime created_utc() const;
  ColorFormat colorFormat() const;
  ByteOrder byteOrder() const;
  
  QVariantMap const as_variant();
  static ptr from_variant(const QVariantMap &map);
  typedef std::chrono::duration<double> Seconds;
  Seconds exposure() const;
  void set_exposure(const Seconds &exposure);
  void overrideByteOrder(ByteOrder byteOrder);
private:
  DPTR
};

#endif // FRAME_H
