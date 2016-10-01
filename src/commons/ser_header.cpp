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
#include "ser_header.h"
#include <map>

using namespace std;

namespace {
map<Frame::ColorFormat, SER_Header::ColorId> color_format_conversion(){
  static map<Frame::ColorFormat, SER_Header::ColorId> _map{
    {Frame::Mono, SER_Header::MONO},
    {Frame::Bayer_RGGB, SER_Header::BAYER_RGGB},
    {Frame::Bayer_GRBG, SER_Header::BAYER_GRBG},
    {Frame::Bayer_GBRG, SER_Header::BAYER_GBRG},
    {Frame::Bayer_BGGR, SER_Header::BAYER_BGGR},
        // TODO: not yet handled
//        BAYER_CYYM = 16,
//        BAYER_YCMY = 17,
//        BAYER_YMCY = 18,
//        BAYER_MYYC = 19,
    {Frame::RGB, SER_Header::RGB},
    {Frame::BGR, SER_Header::BGR},
  };
  return _map;
};
}

size_t SER_Header::frame_size() const
{
  return imageWidth * imageHeight * bytesPerPixel();
}

int SER_Header::channels() const
{
  return (colorId == RGB || colorId == BGR) ? 3 : 1;
}

int SER_Header::bytesPerPixel() const
{
  return channels() * (pixelDepth <= 8 ? 1 : 2);
}

Frame::ColorFormat SER_Header::frame_color_format() const
{
  for(auto format: color_format_conversion()) {
    if(format.second == static_cast<ColorId>(colorId))
      return format.first;
  }
}

void SER_Header::set_color_format(const Frame::ColorFormat& format)
{
  colorId = static_cast<int>(color_format_conversion()[format]);
}

 SER_Timestamp SER_Header::timestamp(const QDateTime& datetime)
{
  static const QDateTime reference{{1, 1, 1}, {0,0,0}};
  return reference.msecsTo(datetime) * 10000;
}

QDateTime SER_Header::qdatetime(const SER_Timestamp& timestamp)
{
  static const QDateTime reference{{1, 1, 1}, {0,0,0}};
  return reference.addMSecs(timestamp / 10000);
}

