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
  ImageData(int width, int height, int bpp, int channels, uint8_t *data) : _width{width}, _height{height}, _bpp{bpp}, _channels{channels}, _data{new uint8_t[size()]}
  {
    std::memcpy(_data, data, size());
  }
  
public:
  static ImageDataPtr create(int width, int height, int bpp, int channels, uint8_t *data) { return std::shared_ptr<ImageData>(new ImageData{width, height, bpp, channels, data}); }
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
