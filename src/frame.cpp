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

#include "frame.h"
using namespace std;

DPTR_IMPL(Frame) {
  Private(uint8_t bpp, ColorFormat colorFormat, const QSize &resolution);
  Private(ColorFormat colorFormat, const cv::Mat &image);
  const QDateTime created_utc;
  const ColorFormat color_format;
  const uint8_t bpp;
  const QSize resolution;
  cv::Mat mat;
  
  static int cv_type(uint8_t bpp, ColorFormat format);
};


int Frame::Private::cv_type(uint8_t bpp, Frame::ColorFormat format)
{
  return CV_MAKETYPE( bpp == 8 ? CV_8U : CV_16U, format == RGB || format == BGR ? 3 : 1);
}



Frame::Private::Private(uint8_t bpp, Frame::ColorFormat colorFormat, const QSize &resolution)
  : created_utc{QDateTime::currentDateTimeUtc()},
  color_format{colorFormat},
  bpp{bpp},
  resolution{resolution},
  mat{resolution.height(), resolution.width(), Private::cv_type(bpp, colorFormat)}
{
}

Frame::Private::Private(Frame::ColorFormat colorFormat, const cv::Mat& image)
  : created_utc{QDateTime::currentDateTimeUtc()},
  color_format{colorFormat},
  bpp{image.depth() == CV_8U || image.depth() == CV_8S ? uint8_t{8} : uint8_t{16}},
  resolution{image.cols, image.rows},
  mat{image}
{
}



Frame::Frame(uint8_t bpp, Frame::ColorFormat colorFormat, const QSize& resolution) : dptr(bpp, colorFormat, resolution)
{
}

Frame::Frame(Frame::ColorFormat colorFormat, const cv::Mat& image) : dptr(colorFormat, image)
{
}


Frame::~Frame()
{
}

uint8_t * Frame::data()
{
  return d->mat.data;
}


cv::Mat Frame::mat() const
{
  return d->mat;
}

QSize Frame::resolution() const
{
  return d->resolution;
}

std::size_t Frame::size() const
{
  return d->mat.total()* d->mat.elemSize();
}

uint8_t Frame::bpp() const
{
  return d->bpp;
}

uint8_t Frame::channels() const
{
  return d->mat.channels();
}

QDateTime Frame::created_utc() const
{
  return d->created_utc;
}

Frame::ColorFormat Frame::colorFormat() const
{
  return d->color_format;
}

