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
#ifndef IMAGE_H
#define IMAGE_H

#include <memory>
#include <cstring>

class ImageData;
typedef std::shared_ptr<ImageData> ImageDataPtr;

class ImageData {
private:
  const int _width;
  const int _height;
  const int _bpp;
  const int _channels;
  uint8_t *_data;
  ImageData(int width, int height, int bpp, int channels, const uint8_t *data) : _width{width}, _height{height}, _bpp{bpp}, _channels{channels}, _data{new uint8_t[size()]}
  {
    std::memcpy(_data, data, size());
  }
  
public:
  static ImageDataPtr create(int width, int height, int bpp, int channels, const uint8_t *data) { return std::shared_ptr<ImageData>(new ImageData{width, height, bpp, channels, data}); }
  ~ImageData() {
    delete [] _data;
  }
 int width() const { return _width; }
 int height() const { return _height; }
 int bpp() const { return _bpp; }
 int channels() const { return _channels; }
 int size() const { return width() * height() * channels() * bytesPerPixel();}
 uint8_t *data() const { return _data; }
 int bytesPerPixel() const { return bpp() <= 8 ? 1 : 2; }
};

#endif
